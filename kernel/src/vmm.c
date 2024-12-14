#include <stdint.h>
#include <limine.h>
#include <mem.h>
#include <vmm.h>
#include <assert.h>
#include <elf.h>

#define MAX_PAGE_TABLE_COUNT 1024
#define MAX_REGIONS   1024


struct region {
    int64_t start;
    uint64_t size;
    uint64_t is_free;

    int64_t next_region_index;
};

static uint64_t page_table_frames;
static uint64_t page_table_bmp_ptr; // Simple bump allocator

static int64_t hhdm_offset = 0;

static struct region regions[MAX_REGIONS];
static uint64_t n_regions = 0;

static uint64_t vmm_cr3()
{
    uint64_t cr3;

    asm __volatile__(
        "mov %%cr3, %0\n"
        : "=r"(cr3)
        :
        :);

    return cr3;
}

static void page_table_init()
{
    page_table_frames = (uint64_t)pmm_alloc_frame(MAX_PAGE_TABLE_COUNT);
    if (page_table_frames == 0) {
        TODO("init page tables failed!");
    }
}

// returns the physical address.
static uint64_t page_table_alloc_frame()
{
    if (page_table_bmp_ptr >= MAX_PAGE_TABLE_COUNT) {
        TODO("we are out of page tables! allocate some more");
    }
    uint64_t page_table_frame = page_table_frames + (page_table_bmp_ptr * 4096);
    page_table_bmp_ptr++;
    return page_table_frame;
}

// find free virtual memory address.
static struct region* region_find_free_virt(uint64_t size)
{
    if (n_regions >= MAX_REGIONS) {
        TODO("n_regions is greater than MAX_REGIONS!\n");
    }

    struct region* region = regions;
    uint64_t found_region_indx = 0;

    while (region->next_region_index != -1)
    {
        if (region->size >= size && region->is_free) {
            break;
        }
        found_region_indx = region->next_region_index;
        region = regions + region->next_region_index;
    }

    if (!region->is_free) {
        ksp("cannot find a free region\n");
        return NULL;
    }

    struct region new_region = {};
    new_region.size = size;
    new_region.start = region->start;
    new_region.is_free = 0;
    new_region.next_region_index = found_region_indx;

    region->start += size;
    region->size  -= size;

    regions[n_regions++] = new_region;

    return regions + (n_regions - 1);
}

static void region_map(struct region* vm_region, uint64_t page_frame)
{
    uint64_t vm_addr = vm_region->start;

    while (vm_addr < vm_region->start + vm_region->size)
    {
        uint64_t p4_offset = (uint64_t)(((uint64_t)vm_addr >> 39) & 0x01ff);
        uint64_t p3_offset = (uint64_t)(((uint64_t)vm_addr >> 30) & 0x01ff);
        uint64_t p2_offset = (uint64_t)(((uint64_t)vm_addr >> 21) & 0x01ff);
        uint64_t p1_offset = (uint64_t)(((uint64_t)vm_addr >> 12) & 0x01ff);

        uint64_t current_p4_frame =  vmm_cr3();
        uint64_t * p4_table  =  (uint64_t*)(current_p4_frame + hhdm_offset);
        uint64_t * p3_table;
        uint64_t * p2_table;
        uint64_t * p1_table;

        if (!(p4_table[p4_offset] & FRAME_PRESENT))
        {
            uint64_t p3_table_frame = page_table_alloc_frame();
            p4_table[p4_offset] = (uint64_t)(p3_table_frame) | FRAME_PRESENT | FRAME_WRITABLE;
            p3_table = (void*)(p3_table_frame + hhdm_offset);
        } else {
            uint64_t p3_table_frame = p4_table[p4_offset] >> 12;
            p3_table = (void*)((p3_table_frame << 12) + hhdm_offset);
        }


        if (!(p3_table[p3_offset] & FRAME_PRESENT))
        {
            uint64_t p2_table_frame = page_table_alloc_frame();
            p3_table[p3_offset] = (uint64_t)(p2_table_frame) | FRAME_PRESENT | FRAME_WRITABLE;
            p2_table = (void*)(p2_table_frame + hhdm_offset);
        } else {
            uint64_t p2_table_frame = p3_table[p3_offset] >> 12;
            p2_table = (void*)((p2_table_frame << 12) + hhdm_offset);
        }

        if (!(p2_table[p2_offset] & FRAME_PRESENT))
        {
            uint64_t p1_table_frame = page_table_alloc_frame();
            p2_table[p2_offset] = (uint64_t)(p1_table_frame) | FRAME_PRESENT | FRAME_WRITABLE;
            p1_table = (void*)(p1_table_frame + hhdm_offset);
        } else {
            uint64_t p1_table_frame = p2_table[p2_offset] >> 12;
            p1_table = (void*)((p1_table_frame << 12) + hhdm_offset);
        }

        if (!(p1_table[p1_offset] & FRAME_PRESENT))
        {
            p1_table[p1_offset] = (uint64_t)(page_frame) | FRAME_PRESENT | FRAME_WRITABLE;
        } else {
            // todo("virtual already mapped!");
        }


        vm_addr = vm_addr + 4096; // move to next page.
        page_frame += 4096;
    }

}

void * vmalloc(uint64_t size)
{
    struct region* free_region = region_find_free_virt(size);
    if (!free_region) return NULL;

    uint64_t n_pages = free_region->size / 4096;
    if ((free_region->size % 4096) > 0)
    {
        n_pages++;
    }
    void* free_pages = pmm_alloc_frame(n_pages);
    if (!free_pages) TODO("cleanup free_pages!");

    region_map(free_region, (uint64_t) free_pages);

    return (void*)free_region->start;
}


static void region_init()
{
    struct region start_region = {
        .start = 0xFFFFF00000000000,
        .size = 0xFFFFF7FFFFFFFFFF - 0xFFFFF00000000000,
        .is_free = 1,
        .next_region_index = -1
    };

    regions[n_regions] = start_region;
    n_regions++;
}


void vmm_init(struct limine_hhdm_request hhdm_request)
{
    hhdm_offset = hhdm_request.response->offset;
    page_table_init();
    region_init();
}
