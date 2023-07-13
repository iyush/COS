
section .multiboot_header

;extern main
;
;MODULEALIGN equ 1<<0
;MEMINFO equ 1<<1
;FLAGS equ MODULEALIGN | MEMINFO
;MAGIC equ 0x1BADB002
;CHECKSUM equ -(MAGIC + FLAGS)
;
;section .mbheader
;align 4
;MultiBootHeader:
;  dd MAGIC
;  dd FLAGS
;  dd CHECKSUM



MODULEALIGN equ 1<<0

MULTIBOOT_MAGIC                  equ 0xe85250d6
GRUB_MULTIBOOT_ARCHITECTURE_I386 equ 0 
MULTIBOOT_HEADER_SIZE            equ header_end - header_start
MULTIBOOT_HEADER_CHECKSUM        equ -(MULTIBOOT_MAGIC + GRUB_MULTIBOOT_ARCHITECTURE_I386 + MULTIBOOT_HEADER_SIZE)

MULTIBOOT_HEADER_TAG_FRAMEBUFFER equ 5
MULTIBOOT_HEADER_TAG_END         equ 0

MULTIBOOT_HEADER_TAG_OPTIONAL    equ 1
FRAMEBUFFER_WIDTH                equ 800
FRAMEBUFFER_HEIGHT               equ 600
FRAMEBUFFER_DEPTH                equ 0

header_start:
        align 4
        dd MULTIBOOT_MAGIC ; magic number
        dd GRUB_MULTIBOOT_ARCHITECTURE_I386 
        dd MULTIBOOT_HEADER_SIZE
        dd MULTIBOOT_HEADER_CHECKSUM

;; TODO: Enable framebuffer mode
;; align 8
;; framebuffer_tag_start:
;;         dw MULTIBOOT_HEADER_TAG_FRAMEBUFFER
;;         dw MULTIBOOT_HEADER_TAG_OPTIONAL
;;         dd framebuffer_tag_end - framebuffer_tag_start
;;         dd FRAMEBUFFER_WIDTH
;;         dd FRAMEBUFFER_HEIGHT
;;         dd FRAMEBUFFER_DEPTH
align 8
framebuffer_tag_end:
        dw MULTIBOOT_HEADER_TAG_END
        dw 0
        dd 8
header_end:
