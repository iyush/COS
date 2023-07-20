#include "./kio.h"
#include "./idt.h"
#include "./io.h"
#include "multiboot.h"

#include "./test/test_kstring.h"

void __assert_fail(const char * assertion, const char * file, unsigned int line, const char * function) {
    ksp("!!! Assertion failed for expression: %s\n", assertion);
    ksp("                  in               : %s[%d:]\n", file, line);
    ksp("                  in function      : %s\n", function);
    while (1) {}
}


uint32_t cpuid() {
    uint32_t cpuid;
    asm __volatile__ (
            "mov $0x80000001, %%eax\n"
            "cpuid\n"
            "mov %%edx, %0;"
            :"=r"(cpuid)
            );
    return cpuid;
}

extern uint64_t MULTIBOOT_TAG_PTR;
extern uint64_t MULTIBOOT_MAGIC_NUMBER;

void c_start() {
    test_kstring_all();

    struct multiboot_tag *tag;
    unsigned size;

    ksp("Magic number: %lx\n", MULTIBOOT_MAGIC_NUMBER);

    if (MULTIBOOT_TAG_PTR & 7)
    {
        ksp ("Unaligned mbi: 0x%lx\n", MULTIBOOT_TAG_PTR);
        return;
    }

    size = *(unsigned *) MULTIBOOT_TAG_PTR;
    ksp ("Announced mbi size 0x%x\n", size);
    for (tag = (struct multiboot_tag *) (MULTIBOOT_TAG_PTR + 8);
            tag->type != MULTIBOOT_TAG_TYPE_END;
            tag = (struct multiboot_tag *) ((multiboot_uint8_t *) tag 
                + ((tag->size + 7) & ~7)))
    {
        ksp ("Tag 0x%x, Size 0x%x\n", tag->type, tag->size);
        switch (tag->type)
        {
            case MULTIBOOT_TAG_TYPE_CMDLINE:
                ksp ("Command line = %s\n",
                        ((struct multiboot_tag_string *) tag)->string);
                break;
            case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
                ksp ("Boot loader name = %s\n",
                        ((struct multiboot_tag_string *) tag)->string);
                break;
            case MULTIBOOT_TAG_TYPE_MODULE:
                ksp ("Module at 0x%x-0x%x. Command line %s\n",
                        ((struct multiboot_tag_module *) tag)->mod_start,
                        ((struct multiboot_tag_module *) tag)->mod_end,
                        ((struct multiboot_tag_module *) tag)->cmdline);
                break;
            case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
                ksp ("mem_lower = %uKB, mem_upper = %uKB\n",
                        ((struct multiboot_tag_basic_meminfo *) tag)->mem_lower,
                        ((struct multiboot_tag_basic_meminfo *) tag)->mem_upper);
                break;
            case MULTIBOOT_TAG_TYPE_BOOTDEV:
                ksp ("Boot device 0x%x,%u,%u\n",
                        ((struct multiboot_tag_bootdev *) tag)->biosdev,
                        ((struct multiboot_tag_bootdev *) tag)->slice,
                        ((struct multiboot_tag_bootdev *) tag)->part);
                break;
            case MULTIBOOT_TAG_TYPE_MMAP:
                {
                    multiboot_memory_map_t *mmap;

                    ksp ("mmap\n");

                    for (mmap = ((struct multiboot_tag_mmap *) tag)->entries;
                            (multiboot_uint8_t *) mmap 
                            < (multiboot_uint8_t *) tag + tag->size;
                            mmap = (multiboot_memory_map_t *) 
                            ((unsigned long) mmap
                             + ((struct multiboot_tag_mmap *) tag)->entry_size))
                        ksp (" base_MULTIBOOT_TAG_PTR = 0x%x%x,"
                                " length = 0x%x%x, type = 0x%x\n",
                                (unsigned) (mmap->addr >> 32),
                                (unsigned) (mmap->addr & 0xffffffff),
                                (unsigned) (mmap->len >> 32),
                                (unsigned) (mmap->len & 0xffffffff),
                                (unsigned) mmap->type);
                }
                break;
        }
    }
    tag = (struct multiboot_tag *) ((multiboot_uint8_t *) tag 
            + ((tag->size + 7) & ~7));

    ksp("Total mbi size 0x%lx\n", (unsigned) tag - MULTIBOOT_TAG_PTR);

    /* Check bit 6 to see if we have a valid memory map */
    //if(!(mbd->flags >> 6 & 0x1)) {
    //    ksp("invalid memory map given by GRUB bootloader");
    //}


    //ksp("mbd p: %p\n", mbd);
    //ksp("mbd: %lx\n", *mbd);

    init_idt();

    while (1) {

    }
}
