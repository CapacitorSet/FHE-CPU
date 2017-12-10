FHE-CPU
=======

A Moxie-based CPU for [fully homomorphically encrypted](https://en.wikipedia.org/wiki/Homomorphic_encryption#Fully_homomorphic_encryption) calculations.

## Installing

The project itself requires Linux and Cmake.

Install the GNU toolchain for Moxie (requires curl, cpio, rpm2cpio).

    cd tools
    ./install.sh

## Usage

For now, it doesn't do much: it parses ELF files, searching for the `.text` section.

Compile a sample file. Note that for whatever reason, compiling in one step doesn't work (not on my machine at least), so you need to call the assembler manually.

    tools/opt/moxielogic/bin/moxie-elf-gcc hello.c -S # Creates hello.s
    tools/opt/moxielogic/bin/moxie-elf-as hello.s -o hello

Compile the codebase.

    cmake .
    make

Run the parser.

    ./FHE-CPU hello # Should exit with code 0