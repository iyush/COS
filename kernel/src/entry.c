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
#include "elf.c"
#include "task.c"


// Set the base revision to 2, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.

__attribute__((used, section(".requests"))) static volatile LIMINE_BASE_REVISION(2);

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.
__attribute__((used, section(".requests"))) static volatile struct limine_framebuffer_request framebuffer_request       = { .id = LIMINE_FRAMEBUFFER_REQUEST, .revision = 0};
__attribute__((used, section(".requests"))) static volatile struct limine_memmap_request memmap_request                 = {LIMINE_MEMMAP_REQUEST};
__attribute__((used, section(".requests"))) static volatile struct limine_kernel_address_request kernel_address_request = {LIMINE_KERNEL_ADDRESS_REQUEST};
__attribute__((used, section(".requests"))) static volatile struct limine_hhdm_request hhdm_request                     = {LIMINE_HHDM_REQUEST};
__attribute__((used, section(".requests"))) static volatile struct limine_kernel_file_request kfile_request             = {LIMINE_KERNEL_FILE_REQUEST};
__attribute__((used, section(".requests"))) static volatile struct limine_module_request module_request                 = {LIMINE_MODULE_REQUEST};


// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.

__attribute__((used, section(".requests_start_marker"))) static volatile LIMINE_REQUESTS_START_MARKER;
__attribute__((used, section(".requests_end_marker"))) static volatile LIMINE_REQUESTS_END_MARKER;

void region_init();
void * vmalloc(size_t size);
void page_table_init();
uint64_t page_table_alloc_frame();


// Halt and catch fire function.
static void hcf(void)
{
    asm("cli");
    for (;;)
    {
        asm("hlt");
    }
}









void task_entry_example1()
{
    while (1) {
        ksp("Hello world!");
    }
}

void task_entry_example2()
{
    while (1) {
        ksp("Goodbye World!");
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
        ksp("framebuffer request failed!\n");
        hcf();
    }
    if (memmap_request.response == NULL)
    {
        ksp("memmap request failed!\n");
        hcf();
    }
    if (kernel_address_request.response == NULL)
    {
        ksp("kernel address request failed!\n");
        hcf();
    }
    if (kfile_request.response == NULL)
    {
        ksp("kernel file request failed!\n");
        hcf();
    }
    if (module_request.response == NULL)
    {
        ksp("module request failed!\n");
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

    char * ptr = vmalloc(1024);
    ptr[0] = 'c';
    ptr[1] = 'p';
    ptr[2] = 'p';


    ASSERT_EQ((u32)module_request.response->module_count, 1); // make sure that we have atleast one file.


    Elf64 elf = elf_parse(module_request.response->modules[0]->address, module_request.response->modules[0]->size);
    ksp("pheader %lx\n", (u64) elf.p_headers);
    ksp("sheader %lx\n", (u64) elf.s_headers);

    ksp("%s\n", ptr);

    // task_init(task_entry_example1);
    // task_init(task_entry_example2);

    // for (int i = 0; i < 1023; i++) {
    //     char * ptr2 = vmalloc(1024);
    //     memset(ptr2, 0, 1024);
    //     ptr2[0] = 'g' + (i % 19);
    //     ptr2[1] = '+';
    //     ptr2[2] = '+';
    //     ksp("%d %lx %s\n", i, (uint64_t)ptr2, ptr2);
    // }

    while(1) {}
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

void panic(char * msg)
{
    ksp("PANIC!!!!!!\n\t %s\n", msg);
    while(1) {}
}
