#ifndef VMM_H
#define VMM_H

#include "stdint.h"

typedef struct {
    u64 start;
    u64 size;
    bool is_writable;
} Region;

typedef struct {
    Region * buf;
    u64 len;
    u64 max_regions;
} RegionList;

void page_table_active_walk_and_print(u64 vm_addr, u64 p4_table_address);

RegionList regionlist_create(u64 max_size);
u64 regionlist_append(RegionList* regions, Region region);
Region region_create(u64 start, u64 size, bool is_writable);

Region* reserve_virt(RegionList * regions, u64 address, u64 size);
void region_map(Region vm_region, u64 p4_address, u64 page_frame);
// void * vmalloc(u64 size);
#endif
