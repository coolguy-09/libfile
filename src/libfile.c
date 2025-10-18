#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

#define ELFMAG "\x7f""ELF"
#define ARMAG "!<arch>\n"

int file_type(const char *path) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) return -1;

    uint8_t hdr[512];
    ssize_t n = read(fd, hdr, sizeof(hdr));
    if (n < 512) {
        close(fd);
        return -2;
    }

    // ELF detection
    if (memcmp(hdr, ELFMAG, 4) == 0) {
        uint8_t elf_class = hdr[4]; // 1 = 32-bit, 2 = 64-bit

        uint16_t type;
        uint64_t entry;
        uint16_t phnum;

        if (elf_class == 1) {
            type  = *(uint16_t *)(hdr + 0x10);
            entry = *(uint32_t *)(hdr + 0x18);
            phnum = *(uint16_t *)(hdr + 0x2C);
        } else if (elf_class == 2) {
            type  = *(uint16_t *)(hdr + 0x10);
            entry = *(uint64_t *)(hdr + 0x18);
            phnum = *(uint16_t *)(hdr + 0x38);
        } else {
            close(fd);
            return 5; // Unknown ELF class
        }

        // Embedded file_interp logic
        lseek(fd, 0, SEEK_SET);
        uint8_t buf[4096];
        ssize_t r;
        while ((r = read(fd, buf, sizeof(buf))) > 0) {
            for (int i = 0; i < r - 16; i++) {
                if (memcmp(&buf[i], "/lib", 4) == 0 || memcmp(&buf[i], "/usr", 4) == 0) {
                    int j = i;
                    while (j < r && buf[j] != 0 && j - i < 255) {
                        j++;
                    }
                    break;
                }
            }
        }

        close(fd);

        if (type == 2) return 1; // ET_EXEC
        if (type == 3 && entry != 0 && phnum > 0 && strstr(path, ".so") == NULL) return 1; // PIE
        if (type == 3) return 2; // Shared object
        if (type == 1) return 3; // Relocatable
        if (type == 4) return 4; // Core dump
        return 5; // Other ELF
    }

    close(fd);

    // Archive detection
    if (memcmp(hdr, ARMAG, 8) == 0) return 6; // AR archive
    if (memcmp(hdr, "\xFD\x37\x7A\x58\x5A\x00", 6) == 0) return 8; // XZ compressed
	if (memcmp(hdr, "\x1F\x8B", 2) == 0) return 9; // GZ compressed
	if (memcmp(hdr, "BZh", 3) == 0) return 10; // BZ2 compressed
	if (memcmp(hdr, "\x04\x22\x4D\x18", 4) == 0) return 11; // LZ4 compressed
	if (memcmp(hdr, "070707", 6) == 0) return 12; // CPIO (ASCII)
	if (memcmp(hdr, "070701", 6) == 0) return 13; // CPIO (NEWC)
	if (memcmp(hdr, "070702", 6) == 0) return 14; // CPIO (CRC)
	if (memcmp(hdr, "\xC7\x71", 2) == 0) return 15; // CPIO (Binary)

    // Text file heuristic
    int ascii = 1;
    for (int i = 0; i < n; i++) {
        if (hdr[i] < 0x09 || (hdr[i] > 0x0D && hdr[i] < 0x20)) {
            ascii = 0;
            break;
        }
    }
    if (ascii) return 7;
	
    return 0; // Unknown file
}

int file_arch(const char *path) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) return -1;

    uint8_t hdr[64];
    ssize_t n = read(fd, hdr, sizeof(hdr));
    close(fd);
    if (n < 64) return -2;

    if (memcmp(hdr, ELFMAG, 4) != 0) return 0; // Not a ELF executable

    uint16_t e_machine = *(uint16_t *)(hdr + 0x12);

    switch (e_machine) {
        case 0x03: return 1; // x86
        case 0x3E: return 2; // x86_64
        case 0x28: return 3; // ARM
        case 0xB7: return 4; // ARM64
        case 0x08: return 5; // MIPS
        case 0xF3: return 6; // RISC-V
        default:   return 7; // Unknown arch
    }
}
