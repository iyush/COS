#include <stdint.h>
#include "gdt.h"

#define GDT_NUM_ENTRIES 10
struct gdt_entry gdt[GDT_NUM_ENTRIES] __attribute__((aligned(8)));
struct gdt_ptr gp __attribute__((aligned(8)));


void gdt_set_gate(
    int num,
    u32 base,
    u32 limit,
    enum gdt_segment_type segment_type,
    enum gdt_descriptor_type descriptor_type, 
    enum gdt_privilege_level dpl,
    enum gdt_limit_granularity granularity) 
{
    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;

    gdt[num].segment_type = segment_type;
    gdt[num].descriptor_type = descriptor_type;
    gdt[num].dpl = dpl;
    gdt[num].present = GDT_PRESENT;

    gdt[num].limit_high = ((limit >> 16) & 0x0F);
    gdt[num].avl = 0;
    gdt[num].l = 1;     // we will always be executing/accessing 64 bit instructions/data.
    gdt[num].db = 0;    // If the L-bit is set, then the D-bit must be cleared
    gdt[num].g = granularity;
    gdt[num].base_high = (base >> 24) & 0xFF;
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
    struct gdt_entry null_entry = {0};
    gdt[0] = null_entry;

    // Kernel Code segment
    gdt_set_gate(5, 0, 0,
        GDT_SEGMENT_TYPE_CODE_EXECUTE_READ_ACCESSED,
        GDT_DESCRIPTOR_TYPE_CODE_OR_DATA,
        GDT_PL_0,
        GDT_LIMIT_GRANULARITY_BYTE);

    // Kernel Data segment
    gdt_set_gate(6, 0, 0,
        GDT_SEGMENT_TYPE_DATA_READ_WRITE_ACCESSED,
        GDT_DESCRIPTOR_TYPE_CODE_OR_DATA,
        GDT_PL_0,
        GDT_LIMIT_GRANULARITY_BYTE);

    // User Code segment
    gdt_set_gate(7, 0, 0,
        GDT_SEGMENT_TYPE_CODE_EXECUTE_READ,
        GDT_DESCRIPTOR_TYPE_CODE_OR_DATA,
        GDT_PL_3,
        GDT_LIMIT_GRANULARITY_BYTE);

    // User Data segment
    gdt_set_gate(8, 0, 0,
        GDT_SEGMENT_TYPE_DATA_READ_WRITE,
        GDT_DESCRIPTOR_TYPE_CODE_OR_DATA,
        GDT_PL_3,
        GDT_LIMIT_GRANULARITY_BYTE);

    // Load GDT
    __asm__ volatile ("lgdt %0" : : "m" (gp));
    bochs_breakpoint();
    
}