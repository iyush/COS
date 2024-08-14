#ifndef _PMM_H
#define _PMM_H
//
// Physical Memory Manager
//

#define PAGE_SIZE      4096

uint64_t _pmm_cr4() {
   uint64_t cr4;

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

#define FRAME_PRESENT     (1 << 0)
#define FRAME_WRITABLE    (1 << 1)
#define FRAME_HUGE        (1 << 7)
#define FRAME_SIZE        4096

void pmm_init(struct limine_memmap_request memmap_request, struct limine_hhdm_request hhdm_request, struct limine_kernel_address_request kernel_address_request);
void * pmm_alloc_frame(uint64_t n_frames);

#endif