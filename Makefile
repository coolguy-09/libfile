include config.mak

SRC = src/libfile.c
OBJ = src/libfile.o

all: libfile.a libfile.so.1.0.0 include/file.h

# Static library
libfile.a: $(OBJ)
	@echo "AR $@"
	@ar rcs $@ $^

# Shared library
libfile.so.1.0.0: $(OBJ)
	@echo "LD $@"
	@$(CC) $(LDFLAGS) -shared -o $@ $^
	@echo "STRIP $@"
	@strip $@

include/file.h: $(SRC)
	@mkdir include
	@echo "H $@"
	@echo "#ifndef FILE_H" > $@
	@echo "#define FILE_H" >> $@
	@echo "" >> $@
	@echo "#include <fcntl.h>" >> $@
	@echo "#include <unistd.h>" >> $@
	@echo "#include <stdint.h>" >> $@
	@echo "#include <string.h>" >> $@
	@echo "" >> $@
	@echo '#define ELFMAG "\x7f""ELF"' >> $@
	@echo '#define ARMAG "!<arch>\\n"' >> $@
	@echo "" >> $@
	@echo "int file_type(const char *path);" >> $@
	@echo "int file_arch(const char *path);" >> $@
	@echo "" >> $@
	@echo "#endif // FILE_H" >> $@

# Object rules
src/%.o: src/%.c
	@echo "CC $@"
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "RM src/*.o *.a *.so.* include/"
	@rm -rf src/*.o *.a *.so.* include/
