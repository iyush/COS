/* Tell the linker that we want an x86_64 ELF64 output file */
OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)

/* We want the symbol _start to be our entry point */
ENTRY(_start)

/* Define the program headers we want so the bootloader gives us the right */
/* MMU permissions */
PHDRS
{
    text     PT_LOAD    FLAGS(0x05); /* Execute + Read */
    rodata   PT_LOAD    FLAGS(0x04); /* Read only */
    data     PT_LOAD    FLAGS(0x06); /* Write + Read */
    dynamic  PT_DYNAMIC FLAGS(0x06); /* Dynamic PHDR for relocations */
}

SECTIONS
{
    /* We wanna be placed in the topmost 2GiB of the address space, for optimisations */
    /* and because that is what the Limine spec mandates. */
    /* Any address in this region will do, but often 0xffffffff80000000 is chosen as */
    /* that is the beginning of the region. */
    . = 0xffffffff80000000;

    _KERNEL_START = .;

    _KERNEL_TXT_START = .;
    .text : {
        *(.text .text.*)
    } :text
    _KERNEL_TXT_END = .;


    /* Move to the next memory page for .rodata */
    . = ALIGN(CONSTANT(MAXPAGESIZE));

    _KERNEL_RO_START = .;
    .rodata : {
        *(.rodata .rodata.*)
    } :rodata
    _KERNEL_RO_END = .;

    /* Move to the next memory page for .data */
    . = ALIGN(CONSTANT(MAXPAGESIZE));

    _KERNEL_DATA_START = .;
    .data : {
        *(.data .data.*)

        /* Place the sections that contain the Limine requests as part of the .data */
        /* output section. */
        KEEP(*(.requests_start_marker))
        KEEP(*(.requests))
        KEEP(*(.requests_end_marker))
    } :data
    _KERNEL_DATA_END = .;

    _KERNEL_DYN_START = .;
    /* Dynamic section for relocations, both in its own PHDR and inside data PHDR */
    .dynamic : {
        *(.dynamic)
    } :data :dynamic
    _KERNEL_DYN_END = .;

    /* NOTE: .bss needs to be the last thing mapped to :data, otherwise lots of */
    /* unnecessary zeros will be written to the binary. */
    /* If you need, for example, .init_array and .fini_array, those should be placed */
    /* above this. */
    _KERNEL_BSS_START = .;
    .bss : {
        *(.bss .bss.*)
        *(COMMON)
    } :data
    _KERNEL_BSS_END = .;

    /* Discard .note.* and .eh_frame* since they may cause issues on some hosts. */
    /* Also discard the program interpreter section since we do not need one. This is */
    /* more or less equivalent to the --no-dynamic-linker linker flag, except that it */
    /* works with ld.gold. */
    /DISCARD/ : {
        *(.eh_frame*)
        *(.note .note.*)
        *(.interp)
    }
    _KERNEL_END = .;
}
