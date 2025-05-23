#include "stdint.h"
#include <limine.h>
#include <assert.h>
#include <elf.h>

#define MAX_PAGE_TABLE_COUNT 1024
#define MAX_REGIONS   1024

typedef struct {
    u64 start;
    u64 size;
} Region;

typedef struct {
    Region * buf;
    u64 len;
    u64 max_regions;
} RegionList;


#define FRAME_PRESENT (1UL << 0) // 1
#define FRAME_WRITABLE (1UL << 1) // 2
#define FRAME_USER (1UL << 2) // 4
#define FRAME_HUGE (1UL << 7)
#define FRAME_NOEXEC (1UL << 63)


typedef union PageTableEntry {
   struct  { 
      u64 is_present: 1;
      u64 is_writable: 1;
      u64 is_user: 1;
      u64 write_through: 1;
      u64 cache_disable: 1;
      u64 is_accessed: 1;
      u64 ignore_1__: 1;
      u64 reserved_must_be_zero___: 1;
      u64 ignore_2__: 3;
      u64 ignored_for_ordinary_paging_but_not_for_hlat_paging___: 1;
      u64 page_frame: 51;
      u64 disable_execute: 1;
   } __attribute__((packed)) bits;
   u64 raw;
} PageTableEntry;


u64 align_up(u64 addr) {
    return (addr + 0xfff) & ~0xfffUL;
}

u64 align_down(u64 addr) {
    return addr & ~0xfffUL;
}


RegionList regionlist_create(PmmAllocator * allocator, u64 max_regions) {
    RegionList rl;

    u64 max_pages = align_up(max_regions * sizeof(Region));
    Frame regionlist_frame = pmm_alloc_frame(allocator, max_pages);

    rl.buf = (Region*)to_higher_half(regionlist_frame);
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

Region region_create(u64 start, u64 size) {
    Region region = {0};
    region.start = start;
    region.size = size;
    return region;
}


// static Region regions[MAX_REGIONS];
// static u64 n_regions = 0;

Frame vmm_cr3()
{
    u64 cr3;

    asm __volatile__(
        "mov %%cr3, %0\n"
        : "=r"(cr3)
        :
        :);

    return frame_create(cr3);
}

Frame page_table_alloc_frame(PmmAllocator* allocator)
{
    return pmm_alloc_frame(allocator, 1);
}

/*
static void page_table_dealloc_frame(PmmAllocator* allocator, PageTableEntry* page_table)
{
    pmm_dealloc_frame(allocator, to_lower_half((u64)page_table), 1);
}
*/

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

    ksp("----- walking the page table -----\n");
    u64 * p4_table  =  (u64*)(p4_table_address);
    ksp("    (0x%lx)p4 table[%ld]: 0x%lx\n", (u64)p4_table, p4_offset, p4_table[p4_offset]);

    u64 p3_table_frame = (p4_table[p4_offset] & ~(1UL << 63)) >> 12;
    u64* p3_table = (void*)to_higher_half(frame_create(p3_table_frame << 12));
    ksp("    (0x%lx)p3 table[%ld]: 0x%lx\n", (u64)p3_table, p3_offset, p3_table[p3_offset]);

    u64 p2_table_frame = (p3_table[p3_offset] & ~(1UL << 63)) >> 12;
    u64* p2_table = (void*)to_higher_half(frame_create(p2_table_frame << 12));
    ksp("    (0x%lx)p2 table[%ld]: 0x%lx\n", (u64)p2_table, p2_offset, p2_table[p2_offset]);

    u64 p1_table_frame = (p2_table[p2_offset] & ~(1UL << 63)) >> 12;
    u64* p1_table = (void*)to_higher_half(frame_create(p1_table_frame << 12));
    ksp("    (0x%lx)p1 table[%ld]: 0x%lx\n", (u64)p1_table, p1_offset, p1_table[p1_offset]);
    ksp("----------------------------------\n");

}

Frame vmm_physical_frame(PageTableEntry* p4_table_address, u64 vm_addr) {
    u64 p4_offset = (u64)(((u64)vm_addr >> 39) & 0x01ff);
    u64 p3_offset = (u64)(((u64)vm_addr >> 30) & 0x01ff);
    u64 p2_offset = (u64)(((u64)vm_addr >> 21) & 0x01ff);
    u64 p1_offset = (u64)(((u64)vm_addr >> 12) & 0x01ff);

    PageTableEntry * p4_table  =  (p4_table_address);

    u64 p3_table_frame = p4_table[p4_offset].bits.page_frame;
    PageTableEntry* p3_table = (void*)to_higher_half(frame_create(p3_table_frame << 12));

    u64 p2_table_frame = p3_table[p3_offset].bits.page_frame;
    PageTableEntry* p2_table = (void*)to_higher_half(frame_create(p2_table_frame << 12));

    u64 p1_table_frame = p2_table[p2_offset].bits.page_frame;
    PageTableEntry* p1_table = (void*)to_higher_half(frame_create(p1_table_frame << 12));

    u64 physical_frame = p1_table[p1_offset].bits.page_frame << 12;
    return frame_create(physical_frame);
}

void page_table_set_zero(PageTableEntry * page_table) {
    u64 max_pages_entries = FRAME_SIZE / sizeof(u64);
    for (u64 i = 0; i < max_pages_entries; i++) {
        page_table[i].raw = 0;
    }
}

void region_map(PmmAllocator* pmm_allocator, Region vm_region, PageTableEntry* p4_address, Frame page_frame, u64 flags)
{
    ASSERT(page_frame.ptr % 0x1000 == 0);
    ASSERT(vm_region.size % 0x1000 == 0);
    ASSERT(vm_region.start % 0x1000 == 0);

    u64 page_flags = flags;
    u64 vm_addr = vm_region.start;
    while (vm_addr < vm_region.start + vm_region.size)
    {
        u64 p4_offset = (u64)(((u64)vm_addr >> 39) & 0x01ff);
        u64 p3_offset = (u64)(((u64)vm_addr >> 30) & 0x01ff);
        u64 p2_offset = (u64)(((u64)vm_addr >> 21) & 0x01ff);
        u64 p1_offset = (u64)(((u64)vm_addr >> 12) & 0x01ff);

        PageTableEntry * p4_table  =  (p4_address);
        PageTableEntry * p3_table;
        PageTableEntry * p2_table;
        PageTableEntry * p1_table;

        if (!p4_table[p4_offset].bits.is_present)
        {
            Frame p3_table_frame = page_table_alloc_frame(pmm_allocator);
            ASSERT(p3_table_frame.ptr);
            p4_table[p4_offset].raw = (u64)(p3_table_frame.ptr) | page_flags;
            p3_table = (PageTableEntry*)to_higher_half(p3_table_frame);
            page_table_set_zero(p3_table);
        } else {
            u64 p3_table_frame = (p4_table[p4_offset].bits.page_frame);
            p4_table[p4_offset].raw |= page_flags;
            p3_table = (PageTableEntry*)to_higher_half(frame_create(p3_table_frame << 12));
        }

        if (!p3_table[p3_offset].bits.is_present)
        {
            Frame p2_table_frame = page_table_alloc_frame(pmm_allocator);
            ASSERT(p2_table_frame.ptr);
            p3_table[p3_offset].raw = (u64)(p2_table_frame.ptr) | page_flags;
            p2_table = (PageTableEntry*)to_higher_half(p2_table_frame);
            page_table_set_zero(p2_table);
        } else {
            u64 p2_table_frame = (p3_table[p3_offset].bits.page_frame);
            p3_table[p3_offset].raw |= page_flags;
            p2_table = (PageTableEntry*)to_higher_half(frame_create(p2_table_frame << 12));
        }

        if (!p2_table[p2_offset].bits.is_present)
        {
            Frame p1_table_frame = page_table_alloc_frame(pmm_allocator);
            ASSERT(p1_table_frame.ptr);
            p2_table[p2_offset].raw = (u64)(p1_table_frame.ptr) | page_flags;
            p1_table = (PageTableEntry*)to_higher_half(p1_table_frame);
            page_table_set_zero(p1_table);
        } else {
            u64 p1_table_frame = (p2_table[p2_offset].bits.page_frame);
            p2_table[p2_offset].raw |= page_flags;
            p1_table = (PageTableEntry*)to_higher_half(frame_create(p1_table_frame << 12));
        }

        if (!p1_table[p1_offset].bits.is_present)
        {
            p1_table[p1_offset].raw = (u64)(page_frame.ptr) | page_flags;
        } else {
            p1_table[p1_offset].raw |= page_flags;
        }

        vm_addr = vm_addr + FRAME_SIZE; // move to next page.
        page_frame.ptr += FRAME_SIZE;
    }
}


bool page_table_is_empty(PageTableEntry* page_table) {
    bool is_empty = true;
    for (int i = 0; i < 512; i++) {
        if (page_table[i].bits.is_present) {
            is_empty = false;
        }
    }
    return is_empty;
}


bool page_table_is_mapped_for_region(Region vm_region, PageTableEntry* p4_table) {
    u64 vm_addr = vm_region.start;
    while (vm_addr < vm_region.start + vm_region.size)
    {
        u64 p4_offset = (u64)(((u64)vm_addr >> 39) & 0x01ff);
        u64 p3_offset = (u64)(((u64)vm_addr >> 30) & 0x01ff);
        u64 p2_offset = (u64)(((u64)vm_addr >> 21) & 0x01ff);
        u64 p1_offset = (u64)(((u64)vm_addr >> 12) & 0x01ff);

        if (p4_table[p4_offset].bits.is_present) {
            PageTableEntry * p3_table = (PageTableEntry*) to_higher_half(frame_create(p4_table[p4_offset].bits.page_frame << 12));
            if (p3_table[p3_offset].bits.is_present) {
                PageTableEntry * p2_table = (PageTableEntry*) to_higher_half(frame_create(p3_table[p3_offset].bits.page_frame << 12));
                if (p2_table[p2_offset].bits.is_present) {
                    PageTableEntry * p1_table = (PageTableEntry*) to_higher_half(frame_create(p2_table[p2_offset].bits.page_frame << 12));
                    if (p1_table[p1_offset].bits.is_present) {
                        return true;
                    }
                }
            }
        }

        vm_addr = vm_addr + FRAME_SIZE; // move to next page.
    }
    return false;
}

void region_unmap(PmmAllocator* pmm_allocator, Region vm_region, PageTableEntry* p4_address) {
    (void)pmm_allocator;
    // CHECK: maybe we need to do invlg???

    u64 vm_addr = vm_region.start;

    while (vm_addr < vm_region.start + vm_region.size)
    {
        u64 p4_offset = (u64)(((u64)vm_addr >> 39) & 0x01ff);
        u64 p3_offset = (u64)(((u64)vm_addr >> 30) & 0x01ff);
        u64 p2_offset = (u64)(((u64)vm_addr >> 21) & 0x01ff);
        u64 p1_offset = (u64)(((u64)vm_addr >> 12) & 0x01ff);

        PageTableEntry * p4_table  =  (p4_address);
        PageTableEntry * p3_table;
        PageTableEntry * p2_table;
        PageTableEntry * p1_table;

        p3_table = (PageTableEntry*)to_higher_half(frame_create(p4_table[p4_offset].bits.page_frame << 12));
        p2_table = (PageTableEntry*)to_higher_half(frame_create(p3_table[p3_offset].bits.page_frame << 12));
        p1_table = (PageTableEntry*)to_higher_half(frame_create(p2_table[p2_offset].bits.page_frame << 12));

        if (p1_table[p1_offset].bits.is_present) {
            p1_table[p1_offset].raw = 0;

            if (page_table_is_empty(p1_table)) {
                // page_table_dealloc_frame(pmm_allocator, p1_table);
                p2_table[p2_offset].raw = 0;
            }

            if (page_table_is_empty(p2_table)) {
                // page_table_dealloc_frame(pmm_allocator, p2_table);
                p3_table[p3_offset].raw = 0;
            }

            if (page_table_is_empty(p3_table)) {
                // page_table_dealloc_frame(pmm_allocator, p3_table);
                p4_table[p4_offset].raw = 0;
            }
        }

        vm_addr = vm_addr + FRAME_SIZE; // move to next page.
    }

    asm __volatile__(
            "mov %%cr3, %%rax\n"
            "mov %%rax, %%cr3\n"
            : : :
            );
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

