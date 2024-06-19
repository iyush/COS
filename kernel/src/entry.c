#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

#include "./kio.h"
#include "./idt.h"
#include "./io.h"
#include <stdint.h>
#include <pmm.h>

extern uint64_t _KERNEL_START;
extern uint64_t _KERNEL_END;
// extern uint64_t * kernel_end;

// Set the base revision to 2, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.

__attribute__((used, section(".requests")))
static volatile LIMINE_BASE_REVISION(2);

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.

__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};
__attribute__((used, section(".requests")))
static volatile struct limine_memmap_request memmap_request = LIMINE_MEMMAP_REQUEST;

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.

__attribute__((used, section(".requests_start_marker")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".requests_end_marker")))
static volatile LIMINE_REQUESTS_END_MARKER;

// GCC and Clang reserve the right to generate calls to the following
// 4 functions even if they are not directly called.
// Implement them as the C specification mandates.
// DO NOT remove or rename these functions, or stuff will eventually break!
// They CAN be moved to a different .c file.

void *memcpy(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t *)s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    if (src > dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if (src < dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

// Halt and catch fire function.
static void hcf(void) {
    asm ("cli");
    for (;;) {
        asm ("hlt");
    }
}


// The following will be our kernel's entry point.
// If renaming _start() to something else, make sure to change the
// linker script accordingly.
void _start(void) {
    // Ensure the bootloader actually understands our base revision (see spec).
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        hcf();
    }

    // Ensure we got a framebuffer.
    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    // Fetch the first framebuffer.
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

    // Note: we assume the framebuffer model is RGB with 32-bit pixels.
    for (size_t i = 0; i < 100; i++) {
        volatile uint32_t *fb_ptr = framebuffer->address;
        fb_ptr[i * (framebuffer->pitch / 4) + i] = 0xffffff;
    }


    if (memmap_request.response == NULL) {
        hcf();
    }

    uint64_t biggest_gap_base = 0;
    uint64_t biggest_gap_length = 0;
    for (uint64_t i = 0; i < memmap_request.response->entry_count; i++) {
        uint64_t type = memmap_request.response->entries[i]->type;
        uint64_t length = memmap_request.response->entries[i]->length;
        uint64_t base = memmap_request.response->entries[i]->base;
        if (type == LIMINE_MEMMAP_USABLE) {
            if (length > biggest_gap_length) {
                biggest_gap_base = base;
                biggest_gap_length = length;
            }
        }

        if (type == LIMINE_MEMMAP_KERNEL_AND_MODULES)
        {
            ksp("kernel_and_modules: 0x%lx 0x%lx\n", 
                    base,
                    length
               );
        }
    }

    ksp("0x%lx 0x%lx\n", 
            biggest_gap_base,
            biggest_gap_length
    );

    /*
     * TODO
    Since biggest_gap_base is a physical_addr we need to get the virt address.
       1. Parse the current page table
       2. Get the entry that the physical address "biggest_gap_base" falls in.
       3. Calculate offset from physical address to physical start -> m_offset;
       4. Use of the offset and virtual start to calculate the virtual address;
    */

    /*
     * After getting the virtual address, create a bitmap at that address which tracks all the memory.
     * Construct a VMM that given a virtual address and the number of pages, adds the entry to the page table.
     * Construct a page table entry for a user space program (stack, text, heap).
     * Set those pages as occupied in PMM (you could do this on the next page fault as well, aka demand paging).
     * Parse the elf binary (passed in as module) and put the text segment.
     * Run the userspace program.
     */


    uint64_t kernel_start = (uint64_t) (&_KERNEL_START);
    uint64_t kernel_end = (uint64_t) (&_KERNEL_END);

    // divide the biggest length into pages.
    uint64_t number_of_pages = biggest_gap_length >> 12;
    uint64_t kernel_text_length = kernel_end - kernel_start;
    uint64_t number_of_kernel_pages = kernel_text_length >> 12;

    ksp("0x%lx\n", kernel_start);
    ksp("0x%lx\n", kernel_end);
    ksp("0x%lx\n", number_of_kernel_pages);
    ksp("0x%lx\n", kernel_text_length);
    ksp("0x%lx\n", number_of_pages);

    init_idt();
    ksp("IDT is initted!\n");

    // add to the first usuable map.
    bitmap.data = (uint64_t *)biggest_gap_base;
    bitmap.size = (number_of_pages >> 6);

    for (uint64_t i = 0; i < bitmap.size; i++) {
        bitmap.data[i] = 0;
    }

    allocate_one_page(bitmap);

    // We're done, just hang...
    hcf();
}

extern void test_kstring_all();

void __assert_fail(const char * assertion, const char * file, unsigned int line, const char * function) {
    ksp("!!! Assertion failed for expression: %s\n", assertion);
    ksp("                  in               : %s[%d:]\n", file, line);
    ksp("                  in function      : %s\n", function);
    while (1) {}
}
