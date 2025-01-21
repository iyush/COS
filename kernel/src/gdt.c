#include <stdint.h>
#include "gdt.h"


static struct TSS tss;
static struct gdt_ptr gp __attribute__((aligned(8)));

void gdt_set_gate(
    int num,
    u32 base,
    u32 limit,
    enum gdt_segment_type segment_type,
    enum gdt_descriptor_type descriptor_type, 
    enum gdt_privilege_level dpl,
    enum gdt_limit_granularity granularity) 
{
    gdt.gdt_entries[num].limit_low = (limit & 0xFFFF);
    gdt.gdt_entries[num].base_low = (base & 0xFFFF);
    gdt.gdt_entries[num].base_middle = (base >> 16) & 0xFF;

    gdt.gdt_entries[num].segment_type = segment_type;
    gdt.gdt_entries[num].descriptor_type = descriptor_type;
    gdt.gdt_entries[num].dpl = dpl;
    gdt.gdt_entries[num].present = GDT_PRESENT;

    gdt.gdt_entries[num].limit_high = ((limit >> 16) & 0x0F);
    gdt.gdt_entries[num].avl = 0;
    gdt.gdt_entries[num].l = 1;     // we will always be executing/accessing 64 bit instructions/data.
    gdt.gdt_entries[num].db = 0;    // If the L-bit is set, then the D-bit must be cleared
    gdt.gdt_entries[num].g = granularity;
    gdt.gdt_entries[num].base_high = (base >> 24) & 0xFF;
}


void gdt_set_tss(
    int num,
    u64 base,
    u32 limit,
    enum gdt_segment_type segment_type,
    enum gdt_limit_granularity granularity) {
    gdt.tss_entries[num].limit_low = (limit & 0xFFFF);
    gdt.tss_entries[num].base_low = (base & 0xFFFF);
    gdt.tss_entries[num].base_middle = (base >> 16) & 0xFF;

    gdt.tss_entries[num].segment_type = segment_type;
    gdt.tss_entries[num].descriptor_type = GDT_DESCRIPTOR_TYPE_SYSTEM;
    gdt.tss_entries[num].dpl = GDT_PL_0;
    gdt.tss_entries[num].present = GDT_PRESENT;

    gdt.tss_entries[num].limit_high = ((limit >> 16) & 0x0F);
    gdt.tss_entries[num].avl = 0;
    gdt.tss_entries[num].l = 0;
    gdt.tss_entries[num].db = 0;
    gdt.tss_entries[num].g = granularity;
    gdt.tss_entries[num].base_high = (base >> 24) & 0xFF;
    gdt.tss_entries[num].base_top = (base >> 32) & 0xFFFFFFFF;
    gdt.tss_entries[num].reserved = 0;
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
void gdt_init(u64 kernel_stack_ptr_address) {
    // Setup GDT pointer and limit
    gp.limit = (sizeof(struct gdt_full)) - 1;
    gp.base = (uint64_t)&gdt;

    memset(&tss, 0, sizeof(struct TSS));
    // tss.i_o_map_base_address = sizeof(TSS);

    // Null descriptor
    struct gdt_entry null_entry = {0};
    gdt.gdt_entries[0] = null_entry;


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

    // TSS segment
    gdt_set_tss(0, (u64)&tss, 0x67, GDT_SEGMENT_TYPE_TSS_64_BIT_AVAILABLE, GDT_LIMIT_GRANULARITY_BYTE);

    // Load GDT
    tss.rsp0 = kernel_stack_ptr_address;
    __asm__ volatile ("lgdt %0" : : "m" (gp));    
    asm volatile("ltr %%ax" : : "a"((GDT_NUM_ENTRIES * 8))); // TSS selector

    for (int i = 0; i < GDT_NUM_ENTRIES; i++) {
        ksp("    (%d) limit_low: %d ", i, gdt.gdt_entries[i].limit_low);
        ksp("base_low: %d ", gdt.gdt_entries[i].base_low);
        ksp("base_middle: %d ", gdt.gdt_entries[i].base_middle);
        ksp("segment_type: %d ", gdt.gdt_entries[i].segment_type);
        ksp("descriptor_type: %d ", gdt.gdt_entries[i].descriptor_type);
        ksp("dpl: %d ", gdt.gdt_entries[i].dpl);
        ksp("present: %d ", gdt.gdt_entries[i].present);
        ksp("limit_high: %d ", gdt.gdt_entries[i].limit_high);
        ksp("avl: %d ", gdt.gdt_entries[i].avl);
        ksp("l: %d ", gdt.gdt_entries[i].l);
        ksp("db: %d ", gdt.gdt_entries[i].db);
        ksp("g: %d ", gdt.gdt_entries[i].g);
        ksp("base_high: %d ", gdt.gdt_entries[i].base_high);
        ksp("\n");
    }
}
