#include <stdint.h>

#define GDT_NUM_ENTRIES 9
struct gdt_entry gdt[GDT_NUM_ENTRIES] __attribute__((aligned(8)));
struct gdt_ptr gp __attribute__((aligned(8)));

void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F);
    gdt[num].granularity |= gran & 0xF0;
    gdt[num].access = access;
}

/*

GDT table address should be a virtual addresses (aka linear address).

we are trying to not rock the boat and do the same thing as limine here:
gdt[0x0000]=<null entry>
gdt[0x0008]=Code segment, base=0x00000000, limit=0x0000ffff, Execute/Read, Non-Conforming, Accessed, 16-bit
gdt[0x0010]=Data segment, base=0x00000000, limit=0x0000ffff, Read/Write, Accessed
gdt[0x0018]=Code segment, base=0x00000000, limit=0xffffffff, Execute/Read, Non-Conforming, Accessed, 32-bit
gdt[0x0020]=Data segment, base=0x00000000, limit=0xffffffff, Read/Write, Accessed
gdt[0x0028]=Code segment, base=0x00000000, limit=0x00000000, Execute/Read, Non-Conforming, Accessed, 64-bit
gdt[0x0030]=Data segment, base=0x00000000, limit=0x00000000, Read/Write, Accessed
gdt[0x0038]=Code segment, base=0x00000000, limit=0x00000000, Execute/Read, Non-Conforming, 64-bit
gdt[0x0040]=Data segment, base=0x00000000, limit=0x00000000, Read/Write


why do this when limine already does this for us? 
 -> because limine maps the gdt table to some other virtual address, that is not
    inside the kernel source virtual address range. This is an issue when switching page tables
    because we will lose those mappings in the new page table. If we lose the mappings, the gdt
    entries are inaccessible and the interrupts will no longer be handled.

*/
void gdt_init(void) {
    // Setup GDT pointer and limit
    gp.limit = (sizeof(struct gdt_entry) * GDT_NUM_ENTRIES) - 1;
    gp.base = (uint64_t)&gdt;

    // Null descriptor
    gdt_set_gate(0, 0, 0, 0, 0);

    // 16-bit Code segment
    gdt_set_gate(1, 0, 0xFFFF,
        0x9A,    // Present=1, Ring=0, Code, Non-conforming, Readable, Accessed
        0x00);   // 16-bit segment

    // Data segment
    gdt_set_gate(2, 0, 0xFFFF,
        0x92,    // Present=1, Ring=0, Data, Read/Write, Accessed
        0x00);   // 16-bit segment

    // 32-bit Code segment
    gdt_set_gate(3, 0, 0xFFFFFFFF,
        0x9A,    // Present=1, Ring=0, Code, Non-conforming, Readable, Accessed
        0xCF);   // 32-bit segment, 4KB granularity

    // Data segment
    gdt_set_gate(4, 0, 0xFFFFFFFF,
        0x92,    // Present=1, Ring=0, Data, Read/Write, Accessed
        0xCF);   // 32-bit segment, 4KB granularity

    // 64-bit Code segment (kernel)
    gdt_set_gate(5, 0, 0,
        0x9A,    // Present=1, Ring=0, Code, Non-conforming, Readable, Accessed
        0x20);   // Long mode code segment

    // Data segment (kernel)
    gdt_set_gate(6, 0, 0,
        0x92,    // Present=1, Ring=0, Data, Read/Write, Accessed
        0x00);   // Normal segment


    // 64-bit Code segment (user mode)
    gdt_set_gate(7, 0, 0,
        0xFA,    // Present=1, Ring=3, Code, Non-conforming, Readable, Accessed
        0x20);   // Long mode code segment

    // Data segment (user mode)
    gdt_set_gate(8, 0, 0,
        0xF2,    // Present=1, Ring=3, Data, Read/Write, Accessed
        0x00);   // Normal segment


    // Load GDT
    __asm__ volatile ("lgdt %0" : : "m" (gp));
}