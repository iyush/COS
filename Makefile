default: build

FLAGS=-nostdlib -ffreestanding -Wall -Wconversion -Wextra -mgeneral-regs-only
QEMU_FLAGS=-cdrom os.iso -m 256


build: kernel.elf

.PHONY: default all build run clean

%.o: kernel/%.asm
	nasm -f elf64 $< -o $@

%.o: kernel/%.c
	toolchain/build/bin/x86_64-elf-gcc ${FLAGS} -c $< -o $@ -g

kernel.elf: multiboot_header.o boot.o entry.o idt.o kio.o pic.o kstring.o io.o
	ld -n -o kernel.elf -T linker.ld $^

os.iso: kernel.elf grub.cfg
	mkdir -p isofiles/boot/grub
	cp grub.cfg isofiles/boot/grub
	cp kernel.elf isofiles/boot/
	grub-mkrescue -o os.iso isofiles

run: build os.iso
	qemu-system-x86_64 ${QEMU_FLAGS} -serial stdio 

# on debugging
# https://gist.github.com/borrrden/3a5488f6a101417297cb43fb1863ebc5
debug: build os.iso
	qemu-system-x86_64 ${QEMU_FLAGS} -serial stdio -s -S

gdb: build os.iso
	gdb kernel.elf -x init.gdb

gf: build os.iso
	gf2 kernel.elf -x init.gdb

bochs: build os.iso
	bochs -q -f .bochsrc

clean: 
	rm -rf *.o
	rm -rf *.elf
	rm -rf *.bin
	rm -rf *.iso
	rm -rf isofiles
        
all: build
