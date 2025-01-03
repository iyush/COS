#!/bin/bash

rm -rf build/
mkdir build/
x86_64-elf-as crt0.asm -o crt0.o
x86_64-elf-gcc $PWD/hello-world/main.c -o build/hello-world.elf -Lcrt0.o
