#include <stdint.h>


// Define GDT table with proper alignment
// Using alignas(0x1000) for extra safety, though 8-byte would be minimum
struct gdt_entry gdt[7] __attribute__((aligned(0x1000)));
struct gdt_ptr gp __attribute__((aligned(8)));

// Function to set a GDT entry
void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F);
    gdt[num].granularity |= gran & 0xF0;
    gdt[num].access = access;
}

// Function to initialize GDT
void gdt_init(void) {
    // Setup GDT pointer and limit
    gp.limit = (sizeof(struct gdt_entry) * 7) - 1;
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

    // 64-bit Code segment
    gdt_set_gate(5, 0, 0,
        0x9A,    // Present=1, Ring=0, Code, Non-conforming, Readable, Accessed
        0x20);   // Long mode code segment

    // Data segment
    gdt_set_gate(6, 0, 0,
        0x92,    // Present=1, Ring=0, Data, Read/Write, Accessed
        0x00);   // Normal segment

    // Load GDT
    __asm__ volatile ("lgdt %0" : : "m" (gp));
}