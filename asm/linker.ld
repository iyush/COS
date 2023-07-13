ENTRY(start)

SECTIONS {
  . = 1M;

  .boot : 
  {
    *(.multiboot_header)
  }

  .bss : 
  {
    *(.bss)
  }

  .text :
  {
    *(.multiboot_header)
  }
}
