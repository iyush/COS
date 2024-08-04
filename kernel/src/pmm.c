#include <stdint.h>
#include <limine.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

uint8_t* bmp;
uint64_t bmp_size;


// GCC and Clang reserve the right to generate calls to the following
// 4 functions even if they are not directly called.
// Implement them as the C specification mandates.
// DO NOT remove or rename these functions, or stuff will eventually break!
// They CAN be moved to a different .c file.

void *memcpy(void *dest, const void *src, size_t n)
{
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    for (size_t i = 0; i < n; i++)
    {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, int c, size_t n)
{
    uint8_t *p = (uint8_t *)s;

    for (size_t i = 0; i < n; i++)
    {
        p[i] = (uint8_t)c;
    }

    return s;
}

void *memmove(void *dest, const void *src, size_t n)
{
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

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
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    for (size_t i = 0; i < n; i++)
    {
        if (p1[i] != p2[i])
        {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

void bmp_set_free(uint64_t page_ptr, uint64_t n_pages)
{
    uint64_t page_start = page_ptr / 4096; 

    uint64_t page = 0;
    uint64_t page_big_index = 0;
    uint64_t page_sma_index = 0;

    for (uint64_t i = 0; i < n_pages; i++)
    {
        page = page_start + i;
        page_big_index = (page / 8);
        page_sma_index = (page % 8);

        bmp[page_big_index] = bmp[page_big_index] & ~(1 << page_sma_index);
    }
}

void bmp_set_used(void* page_ptr, uint64_t n_pages)
{
    uint64_t page_start = (uint64_t)page_ptr / 4096;

    uint64_t page = 0;
    uint64_t page_big_index = 0;
    uint64_t page_sma_index = 0;

    for (uint64_t i = 0; i < n_pages; i++)
    {
        page = page_start + i;
        page_big_index = (page / 8);
        page_sma_index = (page % 8);

        bmp[page_big_index] = bmp[page_big_index] | (1 << page_sma_index);
    }
}

void pmm_init(struct limine_memmap_request memmap_request, struct limine_hhdm_request hhdm_request, struct limine_kernel_address_request kernel_address_request)
{
    uint64_t highest_page_top = 0;
    for (uint64_t i = 0; i < memmap_request.response->entry_count; i++)
    {
        uint64_t length = memmap_request.response->entries[i]->length;
        uint64_t base = memmap_request.response->entries[i]->base;
        if (base + length > highest_page_top)
        {
            highest_page_top = base + length;
        }
    }

    bmp_size = ((highest_page_top + 4096) / 4096) / 8;
    uint64_t biggest_usable_base = 0;
    uint64_t biggest_usable_length = 0;
    for (uint64_t i = 0; i < memmap_request.response->entry_count; i++)
    {
        uint64_t type = memmap_request.response->entries[i]->type;
        uint64_t length = memmap_request.response->entries[i]->length;
        uint64_t base = memmap_request.response->entries[i]->base;


        if (length > bmp_size && type == LIMINE_MEMMAP_USABLE)
        {
            if (length > biggest_usable_length)
            {
                biggest_usable_base = base;
                biggest_usable_length = length;
            }
            ksp("%lx %lx %ld \n", base, length, type);
        }
    }

    ksp("bmp_size %lx can fit in region with base %lx and length %lx\n", bmp_size, biggest_usable_base, biggest_usable_length);

    bmp = (uint8_t *)biggest_usable_base + hhdm_request.response->offset;

    ksp("kernel physical start: %lx\n", kernel_address_request.response->physical_base);
    ksp("kernel virtual start: %lx\n", kernel_address_request.response->virtual_base);

    memset(bmp, 0xff, bmp_size);

    for (uint64_t i = 0; i < memmap_request.response->entry_count; i++)
    {
        uint64_t type = memmap_request.response->entries[i]->type;
        uint64_t length = memmap_request.response->entries[i]->length;
        uint64_t base = memmap_request.response->entries[i]->base;

        if (type == LIMINE_MEMMAP_USABLE)
        {
            // offset as we don't want the bitmap itself to be usable
            if (base == biggest_usable_base)
            {
                bmp_set_free(base + bmp_size, (length - bmp_size) / 4096);
            }
            else
            {
                bmp_set_free(base, length / 4096);
            }
        }
    }
    // for null page.
    bmp[0] = bmp[0] & ~(1 << 0);
}

void * pmm_find_free_page(uint64_t n_pages)
{
    if (n_pages == 0)
    {
        return NULL;
    }

    uint64_t start_page = 0;
    uint64_t end_page = 0;
    
    for (uint64_t i = 0; i < bmp_size; i++)
    {
        for (uint64_t j = 0; j <= 8; j++)
        {
            end_page = i * 8 + j;
            if (((bmp[i] >> j) & 1) == 1)
            {
                start_page = end_page;
            }

            if ((end_page - start_page) > n_pages)
            {
                return (void*)((start_page + 1) * 4096);
            }
        }
    }

    return NULL;
}

void * pmm_alloc_page(uint64_t n_pages)
{
    void * ptr = pmm_find_free_page(n_pages);
    bmp_set_used(ptr, n_pages);
    return ptr;
}