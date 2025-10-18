# libfile
File library for checking types and architecture.

## How to compile libfile?
### First, lets install some packages:
#### For Debian/Ubuntu:
```sh
sudo apt install gcc make binutils
```
#### For Arch:
```sh
pacman -S gcc make binutils
```
### Now while you are in the folder:
```sh
chmod +x configure # Make it executable
./configure 
make # Compile it
```
> [!TIP]
> You can use "./configure ARCH=" to change the architecture of the compiled library. Example:
> ```sh
> ./configure ARCH=i386 # Compile it for x86
> ./configure ARCH=amd64 # Compile it for x64
> ```
