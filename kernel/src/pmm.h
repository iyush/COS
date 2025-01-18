#ifndef _PMM_H
#define _PMM_H
//
// Physical Memory Manager
//

#include "asa_limine.h"

#define PAGE_SIZE      4096

u64 _pmm_cr4() {
   u64 cr4;

    asm __volatile__ (
            "mov %%cr4, %0\n"
            :"=r"(cr4)
            :
            :
          );

    return cr4;
}

#define HIGHER_HALF_BASE 0xffffffff80000000
#define BITMAP_ADDR      0xffffffffA0000000
#define FRAME_SIZE        4096


typedef struct PmmAllocator {
    u8* bmp;
    u64 bmp_size;
} PmmAllocator;


PmmAllocator pmm_init(struct limine_memmap_request memmap_request, struct limine_hhdm_request hhdm_request, struct limine_kernel_address_request kernel_address_request);
void * pmm_alloc_frame(PmmAllocator* allocator, u64 n_frames);

#endif