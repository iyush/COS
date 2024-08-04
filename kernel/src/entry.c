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

extern uint64_t _KERNEL_START;
extern uint64_t _KERNEL_END;
// extern uint64_t * kernel_end;

// Set the base revision to 2, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.

__attribute__((used, section(".requests"))) static volatile LIMINE_BASE_REVISION(2);

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.

__attribute__((used, section(".requests"))) static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0};
__attribute__((used, section(".requests"))) static volatile struct limine_memmap_request memmap_request = {LIMINE_MEMMAP_REQUEST};
__attribute__((used, section(".requests"))) static volatile struct limine_kernel_address_request kernel_address_request = {LIMINE_KERNEL_ADDRESS_REQUEST};
__attribute__((used, section(".requests"))) static volatile struct limine_hhdm_request hhdm_request = {LIMINE_HHDM_REQUEST};

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.

__attribute__((used, section(".requests_start_marker"))) static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".requests_end_marker"))) static volatile LIMINE_REQUESTS_END_MARKER;


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
        hcf();
    }
    if (memmap_request.response == NULL)
    {
        hcf();
    }
    if (kernel_address_request.response == NULL)
    {
        ksp("kernel address request is not known\n");
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
    vmm_init(hhdm_request);

    hcf();
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
