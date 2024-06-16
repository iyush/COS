#include "./kio.h"
#include "./idt.h"
#include "./io.h"
#include "multiboot.h"
#include <stdint.h>

extern void test_kstring_all();

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

static uint64_t MULTIBOOT_TAG_PTR =0;
static uint64_t MULTIBOOT_MAGIC_NUMBER= 0;


struct pml5 {
    uint64_t p: 1;
    uint64_t rw: 1;
    uint64_t us: 1;
    uint64_t pwt: 1;
    uint64_t pcd: 1;
    uint64_t a: 1;
    uint64_t ig: 5;
    uint64_t r: 1;
    uint64_t pml4: 40;
    uint64_t ig2: 11;
    uint64_t xd: 1;
};

struct pml4 {
    uint64_t p: 1;
    uint64_t rw: 1;
    uint64_t us: 1;
    uint64_t pwt: 1;
    uint64_t pcd: 1;
    uint64_t a: 1;
    uint64_t ig: 5;
    uint64_t r: 1;
    uint64_t pdpte: 40;
    uint64_t ig2: 11;
    uint64_t xd: 1;
};


struct pdpte {
    uint64_t p: 1;
    uint64_t rw: 1;
    uint64_t us: 1;
    uint64_t pwt: 1;
    uint64_t pcd: 1;
    uint64_t a: 1;
    uint64_t ig: 5;
    uint64_t r: 1;
    uint64_t pde: 40;
    uint64_t ig2: 11;
    uint64_t xd: 1;
};

struct pdpte_direct {
    uint64_t p: 1;
    uint64_t rw: 1;
    uint64_t us: 1;
    uint64_t pwt: 1;
    uint64_t pcd: 1;
    uint64_t a: 1;
    uint64_t d: 1;
    uint64_t ps: 1;
    uint64_t g: 1;
    uint64_t ig: 2;
    uint64_t r: 1;
    uint64_t pat: 1;
    uint64_t ig2: 17;
    uint64_t physical_addr: 22;
    uint64_t ig3: 7;
    uint64_t protection_key: 4;
    uint64_t xd: 1;
};

struct pde {
    uint64_t p: 1;
    uint64_t rw: 1;
    uint64_t us: 1;
    uint64_t pwt: 1;
    uint64_t pcd: 1;
    uint64_t a: 1;
    uint64_t ig: 1;
    uint64_t ps: 1;
    uint64_t ig2: 3;
    uint64_t r: 1;
    uint64_t pte: 40;
    uint64_t ig3: 11;
    uint64_t xd: 1;
};

struct pte {
    uint64_t p: 1;
    uint64_t rw: 1;
    uint64_t us: 1;
    uint64_t pwt: 1;
    uint64_t pcd: 1;
    uint64_t a: 1;
    uint64_t d: 1;
    uint64_t pat: 1;
    uint64_t g: 1;
    uint64_t ig: 2;
    uint64_t r: 1;
    uint64_t physical_addr: 40;
    uint64_t ig2: 7;
    uint64_t protection_key: 4;
    uint64_t xd: 1;
};

struct pml4* page_table_4;

extern int p4_table;

void dmp_page_table(struct pml4* p4_t) {
    ksp("P4 TABLE 0x%lx\n",    (uint64_t)p4_t);
    ksp("\t p: %d\n",       p4_t->p);
    ksp("\t rw: %d\n",      p4_t->rw);
    ksp("\t us: %d\n",      p4_t->us);
    ksp("\t pwt: %d\n",     p4_t->pwt);
    ksp("\t pcd: %d\n",     p4_t->pcd);
    ksp("\t a: %d\n",       p4_t->a);
    ksp("\t ig: %d\n",      p4_t->ig);
    ksp("\t r: %d\n",       p4_t->r);
    ksp("\t ig2: %d\n",     p4_t->ig2);
    ksp("\t xd: %d\n",      p4_t->xd);

    struct pdpte * p3_t = (struct pdpte*)(uint64_t)(p4_t->pdpte << 12);
    ksp("\t P3 Table - 0x%x\n", p3_t);
    ksp("\t\t p: %d\n",       p3_t->p);
    ksp("\t\t rw: %d\n",      p3_t->rw);
    ksp("\t\t us: %d\n",      p3_t->us);
    ksp("\t\t pwt: %d\n",     p3_t->pwt);
    ksp("\t\t pcd: %d\n",     p3_t->pcd);
    ksp("\t\t a: %d\n",       p3_t->a);
    ksp("\t\t ig: %d\n",      p3_t->ig);
    ksp("\t\t r: %d\n",       p3_t->r);
    ksp("\t\t ig2: %d\n",     p3_t->ig2);
    ksp("\t\t xd: %d\n",      p3_t->xd);

    struct pde * p2_t = (struct pde*)(uint64_t)(p3_t->pde << 12);
    ksp("\t P2 Table - 0x%x\n",   p2_t);
    ksp("\t\t\t p: %d\n",       p2_t->p);
    ksp("\t\t\t rw: %d\n",      p2_t->rw);
    ksp("\t\t\t us: %d\n",      p2_t->us);
    ksp("\t\t\t pwt: %d\n",     p2_t->pwt);
    ksp("\t\t\t pcd: %d\n",     p2_t->pcd);
    ksp("\t\t\t a: %d\n",       p2_t->a);
    ksp("\t\t\t ig: %d\n",      p2_t->ig);
    ksp("\t\t\t ps: %d\n",      p2_t->ps);
    ksp("\t\t\t r: %d\n",       p2_t->r);
    ksp("\t\t\t xd: %d\n",      p2_t->xd);

    struct pte * p1_t = (struct pte*)(uint64_t)(p2_t->pte << 12);
    ksp("\t P1 Table - %d\n",             p1_t);
    ksp("\t\t\t\t p: %d\n",               p1_t->p);
    ksp("\t\t\t\t rw: %d\n",              p1_t->rw);
    ksp("\t\t\t\t us: %d\n",              p1_t->us);
    ksp("\t\t\t\t pwt: %d\n",             p1_t->pwt);
    ksp("\t\t\t\t pcd: %d\n",             p1_t->pcd);
    ksp("\t\t\t\t a: %d\n",               p1_t->a);
    ksp("\t\t\t\t d: %d\n",               p1_t->d);
    ksp("\t\t\t\t pat: %d\n",             p1_t->pat);
    ksp("\t\t\t\t g: %d\n",               p1_t->g);
    ksp("\t\t\t\t r: %d\n",               p1_t->r);
    ksp("\t\t\t\t physical_addr: %d\n",   p1_t->physical_addr);
    ksp("\t\t\t\t protection_key: %d\n",  p1_t->protection_key);
    ksp("\t\t\t\t xd: %d\n",              p1_t->xd);
}

void _start() {
    // page_table_4 = (struct pml4 *) &p4_table;
    while(1) {}
    char * str = "Hello world\n";
    ksp(str);

    //*(page_table_4 + 510) = *page_table_4;

    /*
    
       currently we are operating in a identity kernel, i.e.
       0x0000000000000000-0x000000003fffffff -> 0x000000000000-0x00003fffffff

       we want to half a higher half kernel, i.e
       0xffffff8000000000-0xffffff803fffffff -> 0x000000000000-0x00003fffffff
     */ 

    asm volatile ("xchgw %bx, %bx");
}

void c_start_2() {
 // // //test_kstring_all();

    struct multiboot_tag *tag;
    unsigned size;

    ksp("Magic number: %lx\n", MULTIBOOT_MAGIC_NUMBER);

    if (MULTIBOOT_TAG_PTR & 7)
    {
        ksp ("Unaligned mbi: 0x%lx\n", MULTIBOOT_TAG_PTR);
        return;
    }

    init_idt();

    asm volatile ("xchgw %bx, %bx");

    size = *(unsigned *) MULTIBOOT_TAG_PTR;
    ksp ("Announced mbi size 0x%x\n", size);
    for (tag = (struct multiboot_tag *) (MULTIBOOT_TAG_PTR + 8);
            tag->type != MULTIBOOT_TAG_TYPE_END;
            tag = (struct multiboot_tag *) ((multiboot_uint8_t *) tag 
                + ((tag->size + 7) & (unsigned int)~7)))
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
                        ksp (" base_addr = 0x%x%x,"
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
            + ((tag->size + 7) & (unsigned int)~7));

    ksp("Total mbi size 0x%x\n", (unsigned)( tag - MULTIBOOT_TAG_PTR));


    /* Check bit 6 to see if we have a valid memory map */
    //if(!(mbd->flags >> 6 & 0x1)) {
    //    ksp("invalid memory map given by GRUB bootloader");
    //}


    //ksp("mbd p: %p\n", mbd);
    //ksp("mbd: %lx\n", *mbd);


    while (1) {
    }
}
