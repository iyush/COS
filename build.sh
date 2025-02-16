#!/bin/bash

set -e

rm -rf build/
mkdir -p build/

x86_64-elf-gcc \
        -g -pipe -Wall -Wextra -Wconversion -Wpedantic -Werror -ggdb -fsanitize=undefined -std=c99 \
        -nodefaultlibs -nostdlib -fno-builtin -ffreestanding \
        -fno-stack-protector -fno-stack-check -fno-lto -fPIE \
        -march=x86-64 -mno-80387 -mno-mmx -mno-sse -mno-sse2 \
        -mno-red-zone -I lib/ -I kernel/src -I limine/  -MMD -MP -c kernel/src/entry.c -o build/entry.c.o

nasm -F dwarf -g -f elf64 kernel/src/idt.asm -o build/idt.asm.o

x86_64-elf-ld build/entry.c.o build/idt.asm.o  -m elf_x86_64 -nostdlib -pie -z text -z max-page-size=0x1000 -T kernel/linker.ld -o build/kernel
printf '\x03' | dd of=build/kernel bs=1 count=1 seek=16 conv=notrunc 2> /dev/null

rm -rf iso_root
mkdir -p iso_root/boot
cp build/kernel iso_root/boot/

# build the userland
pushd userland
./build.sh
popd
cp userland/build/hello-world.elf iso_root/boot/


mkdir -p iso_root/boot/limine
cp limine.cfg limine/limine-bios.sys limine/limine-bios-cd.bin limine/limine-uefi-cd.bin iso_root/boot/limine/
mkdir -p iso_root/EFI/BOOT
cp limine/BOOTX64.EFI iso_root/EFI/BOOT/
cp limine/BOOTIA32.EFI iso_root/EFI/BOOT/
xorriso -as mkisofs -b boot/limine/limine-bios-cd.bin \
        -no-emul-boot -boot-load-size 4 -boot-info-table \
        --efi-boot boot/limine/limine-uefi-cd.bin \
        -efi-boot-part --efi-boot-image --protective-msdos-label \
        iso_root -o template.iso 2> /dev/null
./limine/limine bios-install template.iso 2> /dev/null
echo "Finished building!"

