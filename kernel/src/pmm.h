#pragma once
//
// Physical Memory Manager
//

#define PMM_BITMAP_ADDRESS  0xffffffff0000000
#define PMM_BLOCKS_PER_BYTE 8
#define PMM_BLOCK_SIZE      4096 // bytes
#define PMM_BLOCK_ALIGN     PMM_BLOCK_SIZE

#include <assert.h>


struct mem_bitmap {
    uint64_t * data;
    uint64_t size;
};

struct pml4 {
    uint64_t p: 1;
    uint64_t rw: 1;
    uint64_t us: 1;
    uint64_t pwt: 1;
    uint64_t pcd: 1;
    uint64_t a: 1;
    uint64_t ig: 5;
    uint64_t r: 1;
    uint64_t pdpte: 40;
    uint64_t ig2: 11;
    uint64_t xd: 1;
};


struct pdpte {
    uint64_t p: 1;
    uint64_t rw: 1;
    uint64_t us: 1;
    uint64_t pwt: 1;
    uint64_t pcd: 1;
    uint64_t a: 1;
    uint64_t ig: 5;
    uint64_t r: 1;
    uint64_t pde: 40;
    uint64_t ig2: 11;
    uint64_t xd: 1;
};

struct pdpte_direct {
    uint64_t p: 1;
    uint64_t rw: 1;
    uint64_t us: 1;
    uint64_t pwt: 1;
    uint64_t pcd: 1;
    uint64_t a: 1;
    uint64_t d: 1;
    uint64_t ps: 1;
    uint64_t g: 1;
    uint64_t ig: 2;
    uint64_t r: 1;
    uint64_t pat: 1;
    uint64_t ig2: 17;
    uint64_t physical_addr: 22;
    uint64_t ig3: 7;
    uint64_t protection_key: 4;
    uint64_t xd: 1;
};

struct pde {
    uint64_t p: 1;
    uint64_t rw: 1;
    uint64_t us: 1;
    uint64_t pwt: 1;
    uint64_t pcd: 1;
    uint64_t a: 1;
    uint64_t ig: 1;
    uint64_t ps: 1;
    uint64_t ig2: 3;
    uint64_t r: 1;
    uint64_t pte: 40;
    uint64_t ig3: 11;
    uint64_t xd: 1;
};

struct pte {
    uint64_t p: 1;
    uint64_t rw: 1;
    uint64_t us: 1;
    uint64_t pwt: 1;
    uint64_t pcd: 1;
    uint64_t a: 1;
    uint64_t d: 1;
    uint64_t pat: 1;
    uint64_t g: 1;
    uint64_t ig: 2;
    uint64_t r: 1;
    uint64_t physical_addr: 40;
    uint64_t ig2: 7;
    uint64_t protection_key: 4;
    uint64_t xd: 1;
};


struct mem_bitmap bitmap = { 0 };

// maximum size of the physical memory (RAM)
// static uint32_t   pmm_memory_size = 0;
// number of used blocks
// static uint32_t   pmm_used_blocks = 0;
// maximum number of blocks, this will be pmm_memory_size / PMM_BLOCK_SIZE
// static uint32_t   pmm_max_blocks  = 0;
// memory map bit array, where each bit indicates whether a block is free or not.
// static uint32_t * pmm_memory_map = 0;


struct mem_mmap {
    uint64_t v_from;
    uint64_t v_to;

    uint64_t p_from;
    uint64_t p_to;
};

void get_memory_map(struct mem_mmap* mmap) {
    (void)mmap;
    // struct pml4 pml4 = (cr3);
}


// functions that manipulate the bit array
void pmm_mmap_set(int bit) {
    (void)bit;
   //pmm_memory_map[bit / 32] |= 1 << (bit % 32);
}

void pmm_mmap_unset(int bit) {
    (void) bit;
   //pmm_memory_map[bit / 32] &= ~(1 << (bit % 32));
}

bool pmm_mmap_test(int bit) {
    (void) bit;
   //return pmm_memory_map[bit / 32] & (1 << (bit % 32));
   return 1;
}

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




#define CR4_PCICDE (1 << 17)

bool pmm_is_pcicde() {
   return (_pmm_cr4() & CR4_PCICDE);
}


// static uint64_t p4_table[512] __attribute__((aligned (4096))); // pml4
// static uint64_t p3_table[512] __attribute__((aligned (4096))); // pdpte
// static uint64_t p2_table[512] __attribute__((aligned (4096))); // pd
// static uint64_t p1_table[512] __attribute__((aligned (4096))); // pt

#define HIGHER_HALF_BASE 0xffffffff80000000
#define BITMAP_ADDR 0xffffffffA0000000
#define PAGE_PRESENT (1 << 0)
#define PAGE_WRITABLE (1 << 1)
#define PAGE_HUGE (1 << 7)

void pmm_init(struct limine_memmap_request memmap_request, struct limine_hhdm_request hhdm_request, struct limine_kernel_address_request kernel_address_request);
void * pmm_alloc_page(uint64_t n_pages);


/*
void dmp_page_table(struct pml4* p4_t) { 0xffffffff80006000 0xffffffff80007000 0xffffffff80008000 0xffffffff80009000
    ksp("P4 TABLE 0x%lx\n",    (uint64_t)p4_t);
    ksp("\t p: %d\n",       p4_t->p);
    ksp("\t rw: %d\n",      p4_t->rw);
    ksp("\t us: %d\n",      p4_t->us);
    ksp("\t pwt: %d\n",     p4_t->pwt);
    ksp("\t pcd: %d\n",     p4_t->pcd);
    ksp("\t a: %d\n",       p4_t->a);
    ksp("\t ig: %d\n",      p4_t->ig);
    ksp("\t r: %d\n",       p4_t->r);
    ksp("\t ig2: %d\n",     p4_t->ig2);
    ksp("\t xd: %d\n",      p4_t->xd);

    struct pdpte * p3_t = (struct pdpte*)(uint64_t)(p4_t->pdpte << 12);
    ksp("\t P3 Table - 0x%x\n", p3_t);
    ksp("\t\t p: %d\n",       p3_t->p);
    ksp("\t\t rw: %d\n",      p3_t->rw);
    ksp("\t\t us: %d\n",      p3_t->us);
    ksp("\t\t pwt: %d\n",     p3_t->pwt);
    ksp("\t\t pcd: %d\n",     p3_t->pcd);
    ksp("\t\t a: %d\n",       p3_t->a);
    ksp("\t\t ig: %d\n",      p3_t->ig);
    ksp("\t\t r: %d\n",       p3_t->r);
    ksp("\t\t ig2: %d\n",     p3_t->ig2);
    ksp("\t\t xd: %d\n",      p3_t->xd);

    struct pde * p2_t = (struct pde*)(uint64_t)(p3_t->pde << 12);
    ksp("\t P2 Table - 0x%x\n",   p2_t);
    ksp("\t\t\t p: %d\n",       p2_t->p);
    ksp("\t\t\t rw: %d\n",      p2_t->rw);
    ksp("\t\t\t us: %d\n",      p2_t->us);
    ksp("\t\t\t pwt: %d\n",     p2_t->pwt);
    ksp("\t\t\t pcd: %d\n",     p2_t->pcd);
    ksp("\t\t\t a: %d\n",       p2_t->a);
    ksp("\t\t\t ig: %d\n",      p2_t->ig);
    ksp("\t\t\t ps: %d\n",      p2_t->ps);
    ksp("\t\t\t r: %d\n",       p2_t->r);
    ksp("\t\t\t xd: %d\n",      p2_t->xd);

    struct pte * p1_t = (struct pte*)(uint64_t)(p2_t->pte << 12);
    ksp("\t P1 Table - %d\n",             p1_t);
    ksp("\t\t\t\t p: %d\n",               p1_t->p);
    ksp("\t\t\t\t rw: %d\n",              p1_t->rw);
    ksp("\t\t\t\t us: %d\n",              p1_t->us);
    ksp("\t\t\t\t pwt: %d\n",             p1_t->pwt);
    ksp("\t\t\t\t pcd: %d\n",             p1_t->pcd);
    ksp("\t\t\t\t a: %d\n",               p1_t->a);
    ksp("\t\t\t\t d: %d\n",               p1_t->d);
    ksp("\t\t\t\t pat: %d\n",             p1_t->pat);
    ksp("\t\t\t\t g: %d\n",               p1_t->g);
    ksp("\t\t\t\t r: %d\n",               p1_t->r);
    ksp("\t\t\t\t physical_addr: %d\n",   p1_t->physical_addr);
    ksp("\t\t\t\t protection_key: %d\n",  p1_t->protection_key);
    ksp("\t\t\t\t xd: %d\n",              p1_t->xd);
}
*/

