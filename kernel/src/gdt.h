#ifndef __ASA_GDT
#define __ASA_GDT

#include "stdint.h"

enum gdt_privilege_level {
    GDT_PL_0 = 0,
    GDT_PL_1 = 1,
    GDT_PL_2 = 2,
    GDT_PL_3 = 3,
};

enum gdt_segment_type {
    GDT_SEGMENT_TYPE_DATA_READ_ONLY = 0,
    GDT_SEGMENT_TYPE_DATA_READ_ONLY_ACCESSED = 1,
    GDT_SEGMENT_TYPE_DATA_READ_WRITE = 2,
    GDT_SEGMENT_TYPE_DATA_READ_WRITE_ACCESSED = 3,
    GDT_SEGMENT_TYPE_DATA_READ_ONLY_EXPAND_DOWN = 4,
    GDT_SEGMENT_TYPE_DATA_READ_ONLY_EXPAND_DOWN_ACCESSED = 5,
    GDT_SEGMENT_TYPE_DATA_READ_WRITE_EXPAND_DOWN = 6,
    GDT_SEGMENT_TYPE_DATA_READ_WRITE_EXPAND_DOWN_ACCESSED = 7,
    GDT_SEGMENT_TYPE_CODE_EXECUTE_ONLY = 8,
    GDT_SEGMENT_TYPE_CODE_EXECUTE_ONLY_ACCESSED = 9,
    GDT_SEGMENT_TYPE_CODE_EXECUTE_READ = 10,
    GDT_SEGMENT_TYPE_CODE_EXECUTE_READ_ACCESSED = 11,
    GDT_SEGMENT_TYPE_CODE_EXECUTE_ONLY_CONFORMING = 12,
    GDT_SEGMENT_TYPE_CODE_EXECUTE_ONLY_CONFORMING_ACCESSED = 13,
    GDT_SEGMENT_TYPE_CODE_EXECUTE_READ_CONFORMING = 14,
    GDT_SEGMENT_TYPE_CODE_EXECUTE_READ_CONFORMING_ACCESSED = 15,
};

enum gdt_limit_granularity {
    GDT_LIMIT_GRANULARITY_BYTE = 0,
    GDT_LIMIT_GRANULARITY_4KB = 1,
};

enum {
    GDT_NOT_PRESENT = 0,
    GDT_PRESENT = 1,
};

enum gdt_descriptor_type {
    GDT_DESCRIPTOR_TYPE_SYSTEM = 0,
    GDT_DESCRIPTOR_TYPE_CODE_OR_DATA = 1,
};

// GDT entry structure
struct gdt_entry {
    u16 limit_low;
    u16 base_low;
    u8 base_middle;

    u8 segment_type: 4;
    u8 descriptor_type: 1;
    u8 dpl: 2;
    u8 present: 1;

    u8 limit_high: 4;
    u8 avl: 1;
    u8 l: 1;
    u8 db: 1;
    u8 g: 1;

    u8 base_high;
    // u32 base_top;
    // u32 rest;
} __attribute__((packed));

// GDT pointer structure
struct gdt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));


#endif