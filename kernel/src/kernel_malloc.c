// Very basic bump allocator for kernel malloc
#include "stdint.h"
#include <stddef.h>
#include <stdbool.h>

extern u64 _KERNEL_END;

#define KERNEL_HEAP_SIZE (1024 * 1024) // 1 MiB for kernel heap
static u8 *kernel_heap_start = NULL;
static u8 *kernel_heap_end = NULL;
static u8 *kernel_heap_ptr = NULL;

// Block header for each allocation
typedef struct BlockHeader {
    size_t size;
    struct BlockHeader *next;
    bool free;
} BlockHeader;

#define ALIGN8(x) (((x) + 7) & ~7UL)
#define BLOCK_HEADER_SIZE ALIGN8(sizeof(BlockHeader))

static BlockHeader *free_list = NULL;

void kernel_malloc_init() {
    if (!kernel_heap_start) {
        kernel_heap_start = (u8 *)&_KERNEL_END;
        kernel_heap_end = kernel_heap_start + KERNEL_HEAP_SIZE;
        kernel_heap_ptr = kernel_heap_start;
        free_list = NULL;
    }
}

static void split_block(BlockHeader *block, size_t size) {
    if (block->size >= size + BLOCK_HEADER_SIZE + 8) {
        BlockHeader *new_block = (BlockHeader *)((u8 *)block + BLOCK_HEADER_SIZE + size);
        new_block->size = block->size - size - BLOCK_HEADER_SIZE;
        new_block->free = true;
        new_block->next = block->next;
        block->size = size;
        block->next = new_block;
    }
}

void *kernel_malloc(size_t size) {
    size = ALIGN8(size);
    if (!kernel_heap_start) kernel_malloc_init();
    BlockHeader *prev = NULL;
    BlockHeader *curr = free_list;
    // First-fit search
    while (curr) {
        if (curr->free && curr->size >= size) {
            split_block(curr, size);
            curr->free = false;
            return (void *)((u8 *)curr + BLOCK_HEADER_SIZE);
        }
        prev = curr;
        curr = curr->next;
    }
    // No suitable free block, bump heap
    if (kernel_heap_ptr + BLOCK_HEADER_SIZE + size > kernel_heap_end) {
        return NULL;
    }
    BlockHeader *block = (BlockHeader *)kernel_heap_ptr;
    block->size = size;
    block->free = false;
    block->next = NULL;
    kernel_heap_ptr += BLOCK_HEADER_SIZE + size;
    if (prev) {
        prev->next = block;
    } else {
        free_list = block;
    }
    return (void *)((u8 *)block + BLOCK_HEADER_SIZE);
}

// Coalesce adjacent free blocks
static void coalesce_free_list() {
    BlockHeader *curr = free_list;
    while (curr && curr->next) {
        if (curr->free && curr->next->free &&
            (u8 *)curr + BLOCK_HEADER_SIZE + curr->size == (u8 *)curr->next) {
            curr->size += BLOCK_HEADER_SIZE + curr->next->size;
            curr->next = curr->next->next;
        } else {
            curr = curr->next;
        }
    }
}

void kernel_free(void *ptr) {
    if (!ptr) return;
    BlockHeader *block = (BlockHeader *)((u8 *)ptr - BLOCK_HEADER_SIZE);
    block->free = true;
    coalesce_free_list();
}

void kernel_malloc_reset() {
    kernel_heap_ptr = kernel_heap_start;
    free_list = NULL;
}
