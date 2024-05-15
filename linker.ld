/* Entry symbol is not necessary as by default it already
 * searches for 'start' */
/* ENTRY(start) */


SECTIONS {
  .boot : 
  {
    *(.multiboot_header)
  }

  .text :
  {
    *(.text)
  }

  .bss : 
  {
    *(.bss)
  }
}
