#ifndef PMM_H
//
// Physical Memory Manager
//
#define PMM_H

#define PMM_BLOCKS_PER_BYTE 8
#define PMM_BLOCK_SIZE      2 * 1024 * 1024     // in bytes
#define PMM_BLOCK_ALIGN     PMM_BLOCK_SIZE


// maximum size of the physical memory (RAM)
static uint32_t   pmm_memory_size = 0;
// number of used blocks
static uint32_t   pmm_used_blocks = 0;
// maximum number of blocks, this will be pmm_memory_size / PMM_BLOCK_SIZE
static uint32_t   pmm_max_blocks  = 0;
// memory map bit array, where each bit indicates whether a block is free or not.
static uint32_t * pmm_memory_map = 0;


// functions that manipulate the bit array
void pmm_mmap_set(int bit) {
   pmm_memory_map[bit / 32] |= 1 << (bit % 32);
}

void pmm_mmap_unset(int bit) {
   pmm_memory_map[bit / 32] &= ~(1 << (bit % 32));
}

bool pmm_mmap_test(int bit) {
   return pmm_memory_map[bit / 32] & (1 << (bit % 32));
}



// returns the first index of the first free bit
int pmm_mmap_first_free() {
   for (uint32_t block = 0; block < pmm_max_blocks; block++) {

      // optimization
      if (pmm_memory_map[block] == 0xffff_ffff) {
         continue;
      }

      for (uint32_t idx = 0; idx < 32; idx++) {
         int bit = 1 << idx;
         if (!(pmm_memory_map[block] & bit)) {
            return block + idx;
         }
      }
   }
}

void * pmm_alloc_block() {
}


void pmm_free_block(void *) {

}


#endif
