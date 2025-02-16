//
// Physical Memory Manager
//

#include "stdint.h"
#include <limine.h>
#include "stdint.h"
#include <stddef.h>
#include <stdbool.h>

#define FRAME_SIZE 4096UL

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

u64 pmm_frames_total_allocated = 0;


typedef struct PmmAllocator {
    u64* bmp;
    u64 bmp_size;
} PmmAllocator;



// GCC and Clang reserve the right to generate calls to the following
// 4 functions even if they are not directly called.
// Implement them as the C specification mandates.
// DO NOT remove or rename these functions, or stuff will eventually break!
// They CAN be moved to a different .c file.

void *memcpy(void *dest, const void *src, size_t n)
{
    u8 *pdest = (u8 *)dest;
    const u8 *psrc = (const u8 *)src;

    for (size_t i = 0; i < n; i++)
    {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, int c, size_t n)
{
    u8 *p = (u8 *)s;

    for (size_t i = 0; i < n; i++)
    {
        p[i] = (u8)c;
    }

    return s;
}

void *memmove(void *dest, const void *src, size_t n)
{
    u8 *pdest = (u8 *)dest;
    const u8 *psrc = (const u8 *)src;

    if (src > dest)
    {
        for (size_t i = 0; i < n; i++)
        {
            pdest[i] = psrc[i];
        }
    }
    else if (src < dest)
    {
        for (size_t i = n; i > 0; i--)
        {
            pdest[i - 1] = psrc[i - 1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
    const u8 *p1 = (const u8 *)s1;
    const u8 *p2 = (const u8 *)s2;

    for (size_t i = 0; i < n; i++)
    {
        if (p1[i] != p2[i])
        {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

void bmp_set_free(PmmAllocator* allocator, Frame frame_to_free, u64 n_frames)
{
    u64 frame_ptr = frame_to_free.ptr;

    u64 frame_start = frame_ptr / FRAME_SIZE;

    u64 frame = 0;
    u64 frame_big_index = 0;
    u64 frame_sma_index = 0;

    for (u64 i = 0; i < n_frames; i++)
    {
        frame = frame_start + i;
        frame_big_index = (frame / 64);
        frame_sma_index = (frame % 64);

        allocator->bmp[frame_big_index] = allocator->bmp[frame_big_index] & ~(1UL << frame_sma_index);
    }
}

void bmp_set_used(PmmAllocator* allocator, Frame frame_ptr, u64 n_frames)
{
    u64 frame_start = (u64)frame_ptr.ptr / FRAME_SIZE;

    u64 frame = 0;
    u64 frame_big_index = 0;
    u64 frame_sma_index = 0;

    for (u64 i = 0; i < n_frames; i++)
    {
        frame = frame_start + i;
        frame_big_index = (frame / 64);
        frame_sma_index = (frame % 64);

        allocator->bmp[frame_big_index] = allocator->bmp[frame_big_index] | (1UL << frame_sma_index);
    }

    pmm_frames_total_allocated += n_frames;
}

PmmAllocator pmm_init(struct limine_memmap_request memmap_request, struct limine_hhdm_request hhdm_request, struct limine_kernel_address_request kernel_address_request)
{
    PmmAllocator allocator = {0};

    u64 biggest_usable_base = 0;
    u64 highest_frame_top = 0;
    u64 biggest_usable_length = 0;
    for (u64 i = 0; i < memmap_request.response->entry_count; i++)
    {
        u64 type = memmap_request.response->entries[i]->type;
        u64 length = memmap_request.response->entries[i]->length;
        u64 base = memmap_request.response->entries[i]->base;


        if (length > allocator.bmp_size && type == LIMINE_MEMMAP_USABLE)
        {
            if (length > biggest_usable_length)
            {
                biggest_usable_base = base;
                biggest_usable_length = length;
            }

            if (base + length > highest_frame_top)
            {
                highest_frame_top = base + length;
                ksp("---------- highest_frame_top: %lx\n", highest_frame_top);
            }
        }

        char * type_str = "USABLE";
        if      (type == LIMINE_MEMMAP_RESERVED)                { type_str = "RESERVED"; }
        else if (type == LIMINE_MEMMAP_ACPI_RECLAIMABLE)        { type_str = "ACPI_RECLAIMABLE"; }
        else if (type == LIMINE_MEMMAP_ACPI_NVS)                { type_str = "ACPI_NVS"; }
        else if (type == LIMINE_MEMMAP_BAD_MEMORY)              { type_str = "BAD_MEMORY"; }
        else if (type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE)  { type_str = "BOOTLOADER_RECLAIMABLE"; }
        else if (type == LIMINE_MEMMAP_KERNEL_AND_MODULES)      { type_str = "KERNEL_AND_MODULES"; }
        else if (type == LIMINE_MEMMAP_FRAMEBUFFER)             { type_str = "FRAMEBUFFER"; }

        ksp("%lx %lx %s\n", base, length, type_str);
    }

    ksp("----------final------------ highest_frame_top: %lx\n", highest_frame_top);
    allocator.bmp_size = ((highest_frame_top + FRAME_SIZE) / FRAME_SIZE) / 64UL;

    ksp("bmp_size %lx can fit in region with base %lx and length %lx\n", allocator.bmp_size, biggest_usable_base, biggest_usable_length);

    allocator.bmp = (u64 *)biggest_usable_base + hhdm_request.response->offset / 8;

    ksp("kernel physical start: %lx\n", kernel_address_request.response->physical_base);
    ksp("kernel virtual start: %lx\n", kernel_address_request.response->virtual_base);

    for (u64 i = 0; i < allocator.bmp_size; i++)
    {
        allocator.bmp[i] = 0xffffffffffffffff;
    }


    for (u64 i = 0; i < memmap_request.response->entry_count; i++)
    {
        u64 type = memmap_request.response->entries[i]->type;
        u64 length = memmap_request.response->entries[i]->length;
        u64 base = memmap_request.response->entries[i]->base;

        if (type == LIMINE_MEMMAP_USABLE)
        {
            // offset as we don't want the bitmap itself to be usable
            if (base == biggest_usable_base)
            {
                bmp_set_free(&allocator, frame_create(base + allocator.bmp_size), (length - allocator.bmp_size) / FRAME_SIZE);
            }
            else
            {
                bmp_set_free(&allocator, frame_create(base), length / FRAME_SIZE);
            }
        }
    }
    // for null frame.
    allocator.bmp[0] = allocator.bmp[0] | 1;

    return allocator;
}

Frame pmm_find_free_frame(PmmAllocator *allocator, u64 n_frames)
{
    if (n_frames == 0)
    {
        return frame_create(0);
    }


    u64 zero_count = 0;
    u64 zero_idx = 0;
    bool result_found = false;

    for (u64 i = 0; i < allocator->bmp_size; i++)
    {
        u64 super_frame = allocator->bmp[i];
        if (super_frame != 0xffffffffffffffff) {
            for (u64 j = 0; j < 64; j++)
            {
                if (((super_frame >> j) & 1) == 0)
                {
                    zero_count++;
                } else {
                    zero_count = 0;
                }

                if (zero_count >= n_frames) {
                    zero_idx = (i * 64 + j) - (n_frames - 1);
                    result_found = true;
                    goto end;
                }
            }
        } else {
            zero_count = 0;
        }
    }


end:
    if (result_found) {
        return frame_create(zero_idx * FRAME_SIZE);
    } else {
        return frame_create(0);
    }

}

Frame pmm_alloc_frame(PmmAllocator * allocator, u64 n_frames)
{
    Frame frame = pmm_find_free_frame(allocator, n_frames);
    if (frame.ptr) {
        bmp_set_used(allocator, frame, n_frames);
    }
    // ksp("frame 0x%lx asked: %ld total_allocated_frames: %ld max_frames: %ld\n", frame.ptr, n_frames, pmm_frames_total_allocated, allocator->bmp_size * 64);
    return frame;
}

void pmm_dealloc_frame(PmmAllocator * allocator, Frame frame_ptr, u64 n_frame)
{
    bmp_set_free(allocator, frame_ptr, n_frame);
}
