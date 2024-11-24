#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

#include "./kio.h"
#include "./idt.h"
#include "./io.h"
#include <stdint.h>
#include <pmm.h>

#include "./idt.c"
#include "./pic.c"
#include "./kio.c"
#include "./io.c"
#include "./kstring.c"
#include "pmm.c"
#include "vmm.c"
#include "elf.c"


// Set the base revision to 2, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.

__attribute__((used, section(".requests"))) static volatile LIMINE_BASE_REVISION(2);

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.
__attribute__((used, section(".requests"))) static volatile struct limine_framebuffer_request framebuffer_request       = { .id = LIMINE_FRAMEBUFFER_REQUEST, .revision = 0};
__attribute__((used, section(".requests"))) static volatile struct limine_memmap_request memmap_request                 = {LIMINE_MEMMAP_REQUEST};
__attribute__((used, section(".requests"))) static volatile struct limine_kernel_address_request kernel_address_request = {LIMINE_KERNEL_ADDRESS_REQUEST};
__attribute__((used, section(".requests"))) static volatile struct limine_hhdm_request hhdm_request                     = {LIMINE_HHDM_REQUEST};
__attribute__((used, section(".requests"))) static volatile struct limine_kernel_file_request kfile_request             = {LIMINE_KERNEL_FILE_REQUEST};

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.

__attribute__((used, section(".requests_start_marker"))) static volatile LIMINE_REQUESTS_START_MARKER;
__attribute__((used, section(".requests_end_marker"))) static volatile LIMINE_REQUESTS_END_MARKER;

void init_region();
void * vmalloc(size_t size);
void init_page_tables();
uint64_t alloc_page_table_frame();


// Halt and catch fire function.
static void hcf(void)
{
    asm("cli");
    for (;;)
    {
        asm("hlt");
    }
}


// The following will be our kernel's entry point.
// If renaming _start() to something else, make sure to change the
// linker script accordingly.
void _start(void)
{
    // Ensure the bootloader actually understands our base revision (see spec).
    if (LIMINE_BASE_REVISION_SUPPORTED == false)
    {
        hcf();
    }

    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1)
    {
        ksp("framebuffer request failed!\n");
        hcf();
    }
    if (memmap_request.response == NULL)
    {
        ksp("memmap request failed!\n");
        hcf();
    }
    if (kernel_address_request.response == NULL)
    {
        ksp("kernel address request failed!\n");
        hcf();
    }
    if (kfile_request.response == NULL)
    {
        ksp("kernel file request failed!\n");
        hcf();
    }
    init_idt();

    // Fetch the first framebuffer.
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    volatile uint32_t *fb_ptr = framebuffer->address;

    // Note: we assume the framebuffer model is RGB with 32-bit pixels.
    for (size_t i = 0; i < 100; i++)
    {
        fb_ptr[i * (framebuffer->pitch / 4) + i] = 0xffffff;
    }

    pmm_init(memmap_request, hhdm_request, kernel_address_request);


    init_page_tables();
    init_region();
    char * ptr = vmalloc(1024);
    ptr[0] = 'c';
    ptr[1] = 'p';
    ptr[2] = 'p';
    ksp("%s\n", ptr);

    
    char * ptr2 = vmalloc(1024);
    ptr2[0] = 'g';
    ptr2[1] = '+';
    ptr2[2] = '+';
    ksp("%s\n", ptr2);


    // vmm_init(hhdm_request, kernel_address_request, kfile_request);

    hcf();
}

void todo(char* str)
{
    
    ksp("TODO %s\n", str);
    hcf();
}




#define VMALLOC_START 0xFFFFF00000000000LL
#define VMALLOC_END   0xFFFFF7FFFFFFFFFFLL
#define MAX_REGIONS   1024

struct region {
    int64_t start;
    uint64_t size;
    uint64_t is_free;

    int64_t next_region_index;
};

static struct region regions[MAX_REGIONS];
static uint64_t n_regions = 0;

struct region* find_free_vm_region(uint64_t size)
{
    if (n_regions >= MAX_REGIONS) {
        todo("n_regions is greater than MAX_REGIONS\n");
    }

    struct region* region = regions;
    uint64_t found_region_indx = 0;

    ksp("start: %ld\n", regions[0].start);
    ksp("indx: %ld\n", regions[0].next_region_index);

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

void map(struct region* vm_region, uint64_t page_frame)
{
    uint64_t vm_addr = vm_region->start;

    while (vm_addr < vm_region->start + vm_region->size)
    {
        uint64_t p4_offset = (uint64_t)(((uint64_t)vm_addr >> 39) & 0x01ff);
        uint64_t p3_offset = (uint64_t)(((uint64_t)vm_addr >> 30) & 0x01ff);
        uint64_t p2_offset = (uint64_t)(((uint64_t)vm_addr >> 21) & 0x01ff);
        uint64_t p1_offset = (uint64_t)(((uint64_t)vm_addr >> 12) & 0x01ff);

        uint64_t current_p4_frame =  vmm_cr3();
        uint64_t * p4_table  =  (uint64_t*)(current_p4_frame + hhdm_request.response->offset);
        uint64_t * p3_table;
        uint64_t * p2_table;
        uint64_t * p1_table;

        if (!(p4_table[p4_offset] & FRAME_PRESENT))
        {
            uint64_t p3_table_frame = alloc_page_table_frame();
            p4_table[p4_offset] = (uint64_t)(p3_table_frame) | FRAME_PRESENT | FRAME_WRITABLE; 
            p3_table = (void*)(p3_table_frame + hhdm_request.response->offset);
        } else {
            uint64_t p3_table_frame = p4_table[p4_offset] >> 12;
            p3_table = (void*)((p3_table_frame << 12) + hhdm_request.response->offset);
        }


        if (!(p3_table[p3_offset] & FRAME_PRESENT))
        {
            uint64_t p2_table_frame = alloc_page_table_frame();
            p3_table[p3_offset] = (uint64_t)(p2_table_frame) | FRAME_PRESENT | FRAME_WRITABLE; 
            p2_table = (void*)(p2_table_frame + hhdm_request.response->offset);
        } else {
            uint64_t p2_table_frame = p3_table[p3_offset] >> 12;
            p2_table = (void*)((p2_table_frame << 12) + hhdm_request.response->offset);
        }

        if (!(p2_table[p2_offset] & FRAME_PRESENT))
        {
            uint64_t p1_table_frame = alloc_page_table_frame();
            p2_table[p2_offset] = (uint64_t)(p1_table_frame) | FRAME_PRESENT | FRAME_WRITABLE; 
            p1_table = (void*)(p1_table_frame + hhdm_request.response->offset);
        } else {
            uint64_t p1_table_frame = p2_table[p2_offset] >> 12;
            p1_table = (void*)((p1_table_frame << 12) + hhdm_request.response->offset);
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

void * vmalloc(size_t size)
{
    struct region* free_region = find_free_vm_region(size);
    if (!free_region) return NULL;

    
    uint64_t n_pages = free_region->size / 4096;
    if ((free_region->size % 4096) > 0)
    {
        n_pages++;
    }
    void* free_pages = pmm_alloc_frame(n_pages);
    if (!free_pages) todo("cleanup free_pages!");

    map(free_region, (uint64_t) free_pages);

    return (void*)free_region->start;
}


void init_region()
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




extern void test_kstring_all();

void __assert_fail(const char *assertion, const char *file, unsigned int line, const char *function)
{
    ksp("!!! Assertion failed for expression: %s\n", assertion);
    ksp("                  in               : %s[%d:]\n", file, line);
    ksp("                  in function      : %s\n", function);
    while (1)
    {
    }
}




// PAGE TABLES

#define MAX_PAGE_TABLE_COUNT 1024

static uint64_t page_table_frames;
static uint64_t  page_table_bmp_ptr; // Simple bump allocator

void init_page_tables()
{
    page_table_frames = (uint64_t)pmm_alloc_frame(MAX_PAGE_TABLE_COUNT);
    if (page_table_frames == 0) {
        todo("init page tables failed!");
    }
}


// returns the physical address.
uint64_t alloc_page_table_frame() 
{
    if (page_table_bmp_ptr >= MAX_PAGE_TABLE_COUNT) {
        todo("we are out of page tables! allocate some more");
    }
    uint64_t page_table_frame = page_table_frames + (page_table_bmp_ptr * 4096);
    page_table_bmp_ptr++;
    return page_table_frame;
}

void* alloc_page_table()
{
    return (void *)(alloc_page_table_frame() + hhdm_request.response->offset);
}
