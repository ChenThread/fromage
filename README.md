# fromage

fromage is a voxel engine implementation for the PlayStation 1 video game console.

## Compilation (2024)

At this time, fromage is **very** unsupported. You're on your own with this one - I can't promise I will be able to help.

Requirements:

* Linux
* [Wonderful Toolchain](https://wonderful.asie.pl/docs/getting-started/)
* Python 3 and requisite dependencies: python3-numpy, python3-scipy, python3-Pillow

1. Install the MIPS GCC toolchain from Wonderful: `wf-pacman -S toolchain-gcc-mipsel-elf`
2. Clone candyk-psx to be in a directory adjacent to fromage: `cd ..`, `git clone https://github.com/ChenThread/candyk-psx`
3. Compile candyk-psx using the Wonderful toolchain: `cd candyk-psx`, `export PATH=/opt/wonderful/toolchain/gcc-mipsel-elf/bin:$PATH`, `make`
4. Compile Fromage: `cd ../fromage`, `make TYPE=exe REGION=europe`
