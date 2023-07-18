default: build

FLAGS=-nostdlib -ffreestanding -Wall -Wconversion -Wextra -mgeneral-regs-only
        
build: build/kernel.bin

.PHONY: default build run clean

build/multiboot_header.o: asm/multiboot_header.asm
	mkdir -p build
	nasm -f elf64 asm/multiboot_header.asm -o build/multiboot_header.o

build/boot.o: asm/boot.asm
	mkdir -p build
	nasm -f elf64 asm/boot.asm -o build/boot.o

build/kernel_entry.o: kernel/*
	toolchain/build/bin/x86_64-elf-gcc  ${FLAGS} -c kernel/entry.c -o build/kernel_entry.o -g

build/kernel.bin: build/multiboot_header.o build/boot.o build/kernel_entry.o asm/linker.ld
	ld -n -o build/kernel.bin -T asm/linker.ld build/multiboot_header.o build/boot.o build/kernel_entry.o

build/os.iso: build/kernel.bin asm/grub.cfg
	mkdir -p build/isofiles/boot/grub
	cp asm/grub.cfg build/isofiles/boot/grub
	cp build/kernel.bin build/isofiles/boot/
	grub-mkrescue -o build/os.iso build/isofiles

run: build build/os.iso
	qemu-system-x86_64 -serial stdio -cdrom build/os.iso

# on debugging
# https://gist.github.com/borrrden/3a5488f6a101417297cb43fb1863ebc5
debug: build build/os.iso
	qemu-system-x86_64 -cdrom build/os.iso -s -S &
	gdb build/kernel.bin -x init.gdb



clean: 
	rm -rf build

