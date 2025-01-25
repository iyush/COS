#!/bin/bash

rm -rf build/
mkdir build/
x86_64-elf-as crt0.asm -o crt0.o
x86_64-elf-gcc -g \
        -nodefaultlibs -nostdlib -fno-builtin -ffreestanding \
        -I $PWD/../lib/ $PWD/hello-world/main.c  crt0.o  -o build/hello-world.elf
