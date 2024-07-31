set -e
TOOLCHAIN_DIR=./toolchain/build
PATH=./toolchain/build/bin:$PATH

rm -rf build/
mkdir -p build/

x86_64-elf-gcc  -g -O2 -pipe -Wall -Wextra -Wno-missing-field-initializers -std=gnu11 -ffreestanding -fno-stack-protector -fno-stack-check -fno-lto -fPIE -m64 -march=x86-64 -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone -I kernel/src -I limine/  -MMD -MP -c kernel/src/entry.c -o build/entry.c.o
nasm -F dwarf -g -Wall -f elf64 kernel/src/idt.asm -o build/idt.asm.o

ld build/entry.c.o build/idt.asm.o  -m elf_x86_64 -nostdlib -pie -z text -z max-page-size=0x1000 -T kernel/linker.ld -o build/kernel
printf '\x03' | dd of=build/kernel bs=1 count=1 seek=16 conv=notrunc

rm -rf iso_root
mkdir -p iso_root/boot
cp -v build/kernel iso_root/boot/
mkdir -p iso_root/boot/limine
cp -v limine.cfg limine/limine-bios.sys limine/limine-bios-cd.bin limine/limine-uefi-cd.bin iso_root/boot/limine/
mkdir -p iso_root/EFI/BOOT
cp -v limine/BOOTX64.EFI iso_root/EFI/BOOT/
cp -v limine/BOOTIA32.EFI iso_root/EFI/BOOT/
x86_64-elf-xorriso -as mkisofs -b boot/limine/limine-bios-cd.bin \
        -no-emul-boot -boot-load-size 4 -boot-info-table \
        --efi-boot boot/limine/limine-uefi-cd.bin \
        -efi-boot-part --efi-boot-image --protective-msdos-label \
        iso_root -o template.iso
./limine/limine bios-install template.iso