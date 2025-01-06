#include "stdint.h"
#include <limine.h>
#include <mem.h>
#include <vmm.h>
#include <assert.h>
#include <elf.h>

#define MAX_PAGE_TABLE_COUNT 1024
#define MAX_REGIONS   1024


static u64 page_table_frames;
static u64 page_table_bmp_ptr; // Simple bump allocator

static int64_t hhdm_offset = 0;

u64 align_up(u64 addr) {
    return (addr + 0xfff) & ~0xfff;
}

u64 align_down(u64 addr) {
    return addr & ~0xfff;
}


RegionList regionlist_create(u64 max_regions) {
    RegionList rl;

    u64 max_pages = align_up(max_regions * sizeof(Region));
    void* regionlist_frame = pmm_alloc_frame(max_pages);

    rl.buf = (Region*)to_higher_half((u64)regionlist_frame);
    rl.len = 0;
    rl.max_regions = max_regions;
    return rl;
}

u64 regionlist_append(RegionList* rl, Region region) {
    ASSERT(rl->len < rl->max_regions);
    rl->buf[rl->len] = region;
    rl->len++;
    return rl->len - 1;
}

Region region_create(u64 start, u64 size, bool is_writable) {
    Region region = {};
    region.start = start;
    region.size = size;
    region.is_writable = is_writable;
    return region;
}


// static Region regions[MAX_REGIONS];
// static u64 n_regions = 0;

/*
static u64 vmm_cr3()
{
    u64 cr3;

    asm __volatile__(
        "mov %%cr3, %0\n"
        : "=r"(cr3)
        :
        :);

    return cr3;
}
*/

static void page_table_init()
{
    page_table_frames = (u64)pmm_alloc_frame(MAX_PAGE_TABLE_COUNT);
    if (page_table_frames == 0) {
        TODO("init page tables failed!");
    }
}

// returns the physical address.
static u64 page_table_alloc_frame()
{
    if (page_table_bmp_ptr >= MAX_PAGE_TABLE_COUNT) {
        TODO("we are out of page tables! allocate some more");
    }
    u64 page_table_frame = page_table_frames + (page_table_bmp_ptr * 4096);
    page_table_bmp_ptr++;
    return page_table_frame;
}

u64 page_table_create() {
    return to_higher_half(page_table_alloc_frame());
}

/*
// find free virtual memory address.
static Region* region_find_free_virt(RegionList* regions, u64 size)
{
    if (regions->len >= regions->max_regions) {
        TODO("n_regions is greater than max_regions!\n");
    }

    Region* region = regions->buf;
    u64 found_region_indx = 0;

    while (region->next_region_index != -1)
    {
        if (region->size >= size && region->is_free) {
            break;
        }
        found_region_indx = region->next_region_index;
        region = regions->buf + found_region_indx;
    }

    if (!region->is_free) {
        ksp("cannot find a free region\n");
        return NULL;
    }

    Region new_region = {};
    new_region.size = size;
    new_region.start = region->start;
    new_region.is_free = 0;
    new_region.next_region_index = found_region_indx;

    region->start += size;
    region->size  -= size;

    regions->buf[regions->len++] = new_region;

    return regions->buf + (regions->len - 1);
}

Region* reserve_virt(RegionList* regions, u64 address, u64 size)
{
    if (regions->len >= regions->max_regions) {
        TODO("n_regions is greater than MAX_REGIONS!\n");
    }

    Region* region = regions->buf;
    u64 found_region_indx = 0;

    while (region->next_region_index != -1)
    {
        if (region->size >= size && region->is_free) {
            // check if we can fit the requested region inside the free block.
            if (region->start <= address && region->size >= size) {
                break;
            }
        }
        found_region_indx = region->next_region_index;
        region = regions->buf + found_region_indx;
    }

    if (!region->is_free) {
        ksp("cannot find a free region\n");
        return NULL;
    }

    if (!(region->start <= address && region->size >= size)) {
        ksp("cannot find a free region\n");
        return NULL;
    }

    Region new_region = {};
    new_region.size = size;
    new_region.start = region->start;
    new_region.is_free = 0;
    new_region.next_region_index = found_region_indx;

    region->start += size;
    region->size  -= size;

    regions->buf[regions->len++] = new_region;

    return regions->buf + (regions->len - 1);
}
*/

void page_table_active_walk_and_print(u64 vm_addr, u64 p4_table_address) {
    u64 p4_offset = (u64)(((u64)vm_addr >> 39) & 0x01ff);
    u64 p3_offset = (u64)(((u64)vm_addr >> 30) & 0x01ff);
    u64 p2_offset = (u64)(((u64)vm_addr >> 21) & 0x01ff);
    u64 p1_offset = (u64)(((u64)vm_addr >> 12) & 0x01ff);

    u64 * p4_table  =  (u64*)(p4_table_address);

    u64 p3_table_frame = p4_table[p4_offset] >> 12;
    u64* p3_table = (void*)((p3_table_frame << 12) + hhdm_offset);

    u64 p2_table_frame = p3_table[p3_offset] >> 12;
    u64* p2_table = (void*)((p2_table_frame << 12) + hhdm_offset);

    u64 p1_table_frame = p2_table[p2_offset] >> 12;
    u64* p1_table = (void*)((p1_table_frame << 12) + hhdm_offset);

    ksp("p4 table: Stack page entry flags: %lx\n", p4_table[p4_offset] & 0xFFF);
    ksp("p3 table: Stack page entry flags: %lx\n", p3_table[p3_offset] & 0xFFF);
    ksp("p2 table: Stack page entry flags: %lx\n", p2_table[p2_offset] & 0xFFF);
    ksp("p1 table: Stack page entry flags: %lx\n", p1_table[p1_offset] & 0xFFF);

}

void region_map(Region vm_region, u64 p4_address, u64 page_frame)
{
    ASSERT(page_frame % 0x1000 == 0);

    u64 page_flags = FRAME_PRESENT | FRAME_USER;
    if (vm_region.is_writable) {
        page_flags |= FRAME_WRITABLE;
    }

    u64 vm_addr = vm_region.start;

    while (vm_addr < vm_region.start + vm_region.size)
    {
        u64 p4_offset = (u64)(((u64)vm_addr >> 39) & 0x01ff);
        u64 p3_offset = (u64)(((u64)vm_addr >> 30) & 0x01ff);
        u64 p2_offset = (u64)(((u64)vm_addr >> 21) & 0x01ff);
        u64 p1_offset = (u64)(((u64)vm_addr >> 12) & 0x01ff);

        u64 * p4_table  =  (u64*)(p4_address);
        u64 * p3_table;
        u64 * p2_table;
        u64 * p1_table;

        if (!(p4_table[p4_offset] & FRAME_PRESENT))
        {
            u64 p3_table_frame = page_table_alloc_frame();
            p4_table[p4_offset] = (u64)(p3_table_frame) | page_flags;
            p3_table = (void*)(p3_table_frame + hhdm_offset);
        } else {
            u64 p3_table_frame = p4_table[p4_offset] >> 12;
            p3_table = (void*)((p3_table_frame << 12) + hhdm_offset);
        }


        if (!(p3_table[p3_offset] & FRAME_PRESENT))
        {
            u64 p2_table_frame = page_table_alloc_frame();
            p3_table[p3_offset] = (u64)(p2_table_frame) | page_flags;
            p2_table = (void*)(p2_table_frame + hhdm_offset);
        } else {
            u64 p2_table_frame = p3_table[p3_offset] >> 12;
            p2_table = (void*)((p2_table_frame << 12) + hhdm_offset);
        }

        if (!(p2_table[p2_offset] & FRAME_PRESENT))
        {
            u64 p1_table_frame = page_table_alloc_frame();
            p2_table[p2_offset] = (u64)(p1_table_frame) | page_flags;
            p1_table = (void*)(p1_table_frame + hhdm_offset);
        } else {
            u64 p1_table_frame = p2_table[p2_offset] >> 12;
            p1_table = (void*)((p1_table_frame << 12) + hhdm_offset);
        }

        if (!(p1_table[p1_offset] & FRAME_PRESENT))
        {
            p1_table[p1_offset] = (u64)(page_frame) | page_flags;
        } else {
            // todo("virtual already mapped!");
        }


        vm_addr = vm_addr + 4096; // move to next page.
        page_frame += 4096;
    }

}

/*

// TODO: do not use this until this is flushed out.
void * vmalloc_(RegionList* regions,  u64 size)
{
    Region* free_region = region_find_free_virt(regions, size);
    if (!free_region) return NULL;

    u64 n_pages = free_region->size / 4096;
    if ((free_region->size % 4096) > 0)
    {
        n_pages++;
    }
    void* free_pages = pmm_alloc_frame(n_pages);
    if (!free_pages) TODO("cleanup free_pages!");

    region_map(*free_region, vmm_cr3() + hhdm_offset, (u64) free_pages);

    return (void*)free_region->start;
}

*/

// TODO: Regions should be independent per task. Currently it is assuming we are initing a kernel region.
// static void region_init_()
// {
//     Region start_region = {
//         .start = 0xFFFFF00000000000,
//         .size = 0xFFFFF7FFFFFFFFFF - 0xFFFFF00000000000,
//         .is_free = 1,
//         .next_region_index = -1
//     };

//     regions[n_regions] = start_region;
// }


void vmm_init(struct limine_hhdm_request hhdm_request)
{
    hhdm_offset = hhdm_request.response->offset;
    page_table_init();
    // region_init();
}
