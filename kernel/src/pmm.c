#include "stdint.h"
#include <limine.h>
#include "stdint.h"
#include <stddef.h>
#include <stdbool.h>
#include <pmm.h>

u8* bmp;
u64 bmp_size;


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

void bmp_set_free(u64 frame_ptr, u64 n_frames)
{
    u64 frame_start = frame_ptr / FRAME_SIZE;

    u64 frame = 0;
    u64 frame_big_index = 0;
    u64 frame_sma_index = 0;

    for (u64 i = 0; i < n_frames; i++)
    {
        frame = frame_start + i;
        frame_big_index = (frame / 8);
        frame_sma_index = (frame % 8);

        bmp[frame_big_index] = bmp[frame_big_index] & ~(1 << frame_sma_index);
    }
}

void bmp_set_used(void* frame_ptr, u64 n_frames)
{
    u64 frame_start = (u64)frame_ptr / FRAME_SIZE;

    u64 frame = 0;
    u64 frame_big_index = 0;
    u64 frame_sma_index = 0;

    for (u64 i = 0; i < n_frames; i++)
    {
        frame = frame_start + i;
        frame_big_index = (frame / 8);
        frame_sma_index = (frame % 8);

        bmp[frame_big_index] = bmp[frame_big_index] | (1 << frame_sma_index);
    }
}

void pmm_init(struct limine_memmap_request memmap_request, struct limine_hhdm_request hhdm_request, struct limine_kernel_address_request kernel_address_request)
{
    u64 highest_frame_top = 0;
    for (u64 i = 0; i < memmap_request.response->entry_count; i++)
    {
        u64 length = memmap_request.response->entries[i]->length;
        u64 base = memmap_request.response->entries[i]->base;
        if (base + length > highest_frame_top)
        {
            highest_frame_top = base + length;
        }
    }

    bmp_size = ((highest_frame_top + FRAME_SIZE) / FRAME_SIZE) / 8;
    u64 biggest_usable_base = 0;
    u64 biggest_usable_length = 0;
    for (u64 i = 0; i < memmap_request.response->entry_count; i++)
    {
        u64 type = memmap_request.response->entries[i]->type;
        u64 length = memmap_request.response->entries[i]->length;
        u64 base = memmap_request.response->entries[i]->base;


        if (length > bmp_size && type == LIMINE_MEMMAP_USABLE)
        {
            if (length > biggest_usable_length)
            {
                biggest_usable_base = base;
                biggest_usable_length = length;
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

    ksp("bmp_size %lx can fit in region with base %lx and length %lx\n", bmp_size, biggest_usable_base, biggest_usable_length);

    bmp = (u8 *)biggest_usable_base + hhdm_request.response->offset;

    ksp("kernel physical start: %lx\n", kernel_address_request.response->physical_base);
    ksp("kernel virtual start: %lx\n", kernel_address_request.response->virtual_base);

    memset(bmp, 0xff, bmp_size);

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
                bmp_set_free(base + bmp_size, (length - bmp_size) / FRAME_SIZE);
            }
            else
            {
                bmp_set_free(base, length / FRAME_SIZE);
            }
        }
    }
    // for null frame.
    bmp[0] = bmp[0] & ~(1 << 0);
}

void * pmm_find_free_frame(u64 n_frames)
{
    if (n_frames == 0)
    {
        return NULL;
    }

    u64 start_frame = 0;
    u64 end_frame = 0;

    for (u64 i = 0; i < bmp_size; i++)
    {
        for (u64 j = 0; j <= 8; j++)
        {
            end_frame = i * 8 + j;
            if (((bmp[i] >> j) & 1) == 1)
            {
                start_frame = end_frame;
            }

            if ((end_frame - start_frame) > n_frames)
            {
                return (void*)((start_frame + 1) * FRAME_SIZE);
            }
        }
    }

    return NULL;
}

void * pmm_alloc_frame(u64 n_frames)
{
    // ksp("nframes %ld\n", n_frames);
    void * ptr = pmm_find_free_frame(n_frames);
    if (ptr) {
        bmp_set_used(ptr, n_frames);
    }
    return ptr;
}

void pmm_dealloc_frame(void* ptr, u64 n_frame)
{
    bmp_set_free((u64)ptr, n_frame);
}
