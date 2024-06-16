default: build

FLAGS= \
    -Wall \
    -Wextra \
    -std=gnu11 \
    -ffreestanding \
    -fno-stack-protector \
    -fno-stack-check \
    -fno-lto \
    -fPIE \
    -m64 \
    -march=x86-64 \
    -mno-80387 \
    -mno-mmx \
    -mno-sse \
    -mno-sse2 \
    -mno-red-zone

QEMU_FLAGS=-cdrom os.iso -m 256
LINKER_FLAGS=-n -pie -nostdlib -z text -z max-page-size=0x1000 -T linker.ld -m elf_x86_64


build: kernel.elf

.PHONY: default all build run clean

%.o: kernel/%.asm
	nasm -f elf64 $< -o $@

%.o: kernel/%.c
	toolchain/build/bin/x86_64-elf-gcc ${FLAGS} -c $< -o $@ -g

kernel.elf: multiboot_header.o entry.o idt.o kio.o pic.o kstring.o io.o
	ld ${LINKER_FLAGS} -o kernel.elf  $^

os.iso: kernel.elf limine/limine
	rm -rf iso_root
	mkdir -p iso_root/boot
	cp -v kernel.elf iso_root/boot/
	mkdir -p iso_root/boot/limine
	cp -v limine.cfg limine/limine-bios.sys limine/limine-bios-cd.bin limine/limine-uefi-cd.bin iso_root/boot/limine/
	mkdir -p iso_root/EFI/BOOT
	cp -v limine/BOOTX64.EFI iso_root/EFI/BOOT/
	cp -v limine/BOOTIA32.EFI iso_root/EFI/BOOT/
	xorriso -as mkisofs -b boot/limine/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o os.iso
	./limine/limine bios-install os.iso
	rm -rf iso_root

limine/limine:
	rm -rf limine
	git clone https://github.com/limine-bootloader/limine.git --branch=v7.x-binary --depth=1
	$(MAKE) -C limine

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
