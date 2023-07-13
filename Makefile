default: build
        
build: build/kernel.bin

.PHONY: default build run clean

build/multiboot_header.o: asm/multiboot_header.asm
	mkdir -p build
	nasm -f elf64 asm/multiboot_header.asm -o build/multiboot_header.o

build/boot.o: asm/boot.asm
	mkdir -p build
	nasm -f elf64 asm/boot.asm -o build/boot.o

build/kernel_entry.o: kernel/entry.c
	gcc -nostdlib -c kernel/entry.c -o build/kernel_entry.o -g

build/kernel.bin: build/multiboot_header.o build/boot.o build/kernel_entry.o asm/linker.ld
	ld -n -o build/kernel.bin -T asm/linker.ld build/multiboot_header.o build/boot.o build/kernel_entry.o

build/os.iso: build/kernel.bin asm/grub.cfg
	mkdir -p build/isofiles/boot/grub
	cp asm/grub.cfg build/isofiles/boot/grub
	cp build/kernel.bin build/isofiles/boot/
	grub-mkrescue -o build/os.iso build/isofiles

run: build build/os.iso
	qemu-system-x86_64 -cdrom build/os.iso -chardev stdio,id=seabios -device isa-debugcon,iobase=0x402,chardev=seabios

clean: 
	rm -rf build

