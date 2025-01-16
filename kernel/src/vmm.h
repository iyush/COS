#ifndef VMM_H
#define VMM_H

#include "stdint.h"

typedef struct {
    u64 start;
    u64 size;
} Region;

typedef struct {
    Region * buf;
    u64 len;
    u64 max_regions;
} RegionList;


enum frame_flags : u64 {
   FRAME_PRESENT = (1 << 0), // 1
   FRAME_WRITABLE = (1 << 1), // 2
   FRAME_USER = (1 << 2), // 4
   FRAME_HUGE = (1 << 7),
   FRAME_NOEXEC = (1UL << 63)
};

void page_table_active_walk_and_print(u64 vm_addr, u64 p4_table_address);

RegionList regionlist_create(u64 max_size);
u64 regionlist_append(RegionList* regions, Region region);
Region region_create(u64 start, u64 size);

Region* reserve_virt(RegionList * regions, u64 address, u64 size);


void region_map(Region vm_region, u64 p4_address, u64 page_frame, u64 flags);

u64 vmm_cr3();
u64 vmm_physical_frame(u64 p4_address, u64 virtual_address);

// void * vmalloc(u64 size);
#endif
