#include "stdint.h"
#include <stddef.h>
#include <stdbool.h>

#include "./kio.h"
#include "./idt.h"
#include "./io.h"
#include "stdint.h"
#include <pmm.h>
#include "bochs.h"

#include "./idt.c"
#include "./pic.c"
#include "./kio.c"
#include "./io.c"
#include "./kstring.c"
#include "pmm.c"
#include "vmm.c"
#include "elf.c"
#include "task.c"
#include "asa_limine.h"


#define MAX_REGION_LIST_TASK 1024

extern u8 _KERNEL_START;
extern u8 _KERNEL_END;


extern void set_page_table_and_jump(u64 page_table_frame, u64 stack_address, u64 entry_point);


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
    volatile u32 *fb_ptr = framebuffer->address;

    // Note: we assume the framebuffer model is RGB with 32-bit pixels.
    for (size_t i = 0; i < 100; i++)
    {
        fb_ptr[i * (framebuffer->pitch / 4) + i] = 0xffffff;
    }

    pmm_init(memmap_request, hhdm_request, kernel_address_request);
    vmm_init(hhdm_request);

    ASSERT_EQ((u32)module_request.response->module_count, 1); // make sure that we have atleast one file.

    void* elf_module_start = module_request.response->modules[0]->address; 
    u64 elf_module_size = module_request.response->modules[0]->size; 

    Elf64 elf = elf_parse(elf_module_start, elf_module_size);

    u64 page_table_address = page_table_create();
    RegionList region_list = regionlist_create(MAX_REGION_LIST_TASK);

    for (u64 i = 0; i < elf.p_headers_len; i++) {
        Elf64_Phdr pheader = elf.p_headers[i];
        (void) page_table_address;
        if (pheader.p_type == PT_LOAD) {
            ksp("pheader.p_memsz: %lx\n", pheader.p_memsz);
            ksp("pheader.p_vaddr: %lx\n", pheader.p_vaddr);
            ksp("pheader.p_offset: %lx\n", pheader.p_offset);
            ksp("pheader.p_flags: %x\n", pheader.p_flags);

            pheader.p_vaddr = align_down(pheader.p_vaddr);
            pheader.p_memsz = align_up(pheader.p_memsz);
            pheader.p_offset = align_down(pheader.p_offset);

            ASSERT(pheader.p_vaddr % 0x1000 == 0);
            ASSERT(pheader.p_offset % 0x1000 == 0);

            // reserve the virtual address;
            Region region = region_create(pheader.p_vaddr, pheader.p_memsz, false, false, -1);

            if (pheader.p_flags & PF_R) region.is_writable = false;
            if (pheader.p_flags & PF_W) region.is_writable = true;


            regionlist_append(&region_list, region);
            region_map(region, page_table_address, to_lower_half(pheader.p_offset + (u64)elf_module_start));
        }
    }

    // for (u64 i = 0; i < elf.s_headers_len; i++) {
    //     Elf64_Shdr sheader = elf.s_headers[i];
    //     (void) page_table_address;
    //     if (sheader.sh_type == SHT_PROGBITS) {
    //         ksp("sheader.sh_size: %lx\n", sheader.sh_size);
    //         ksp("sheader.sh_addr: %lx\n", sheader.sh_addr);
    //         ksp("sheader.sh_offset: %lx\n", sheader.sh_offset);

    //         sheader.sh_addr = align_down(sheader.sh_addr);
    //         sheader.sh_size = align_up(sheader.sh_size);
    //         sheader.sh_offset = align_down(sheader.sh_offset);

    //         ASSERT(sheader.sh_addr % 0x1000 == 0);
    //         ASSERT(sheader.sh_offset % 0x1000 == 0);

    //         // reserve the virtual address;
    //         Region region = region_create(sheader.sh_addr, sheader.sh_size, false, -1);
    //         regionlist_append(&region_list, region);
    //         region_map(region, page_table_address, to_lower_half(sheader.sh_offset + (u64)elf_module_start));
    //     }
    // }

    ksp("pheader %lx\n", (u64) elf.p_headers);
    ksp("sheader %lx\n", (u64) elf.s_headers);
    ksp("entry %lx\n", (u64) elf.header.e_entry);
    ksp("page_table_address %lx\n", page_table_address);
    ksp("KERNEL_START %lx\n", (u64)&_KERNEL_START);
    ksp("KERNEL_END %lx\n", (u64)&_KERNEL_END);

    // IMPORTANT: map the kernel on higher half.
    u64 kernel_size = align_up((u64)&_KERNEL_END - (u64)&_KERNEL_START);
    Region kernel_region = region_create(kernel_address_request.response->virtual_base, kernel_size, false, false, -1);
    regionlist_append(&region_list, kernel_region);
    region_map(kernel_region, page_table_address, kernel_address_request.response->physical_base);

    // IMPORTANT: create a stack for the executable
    u64 stack_size = 0x100000; // 1Mib;
    void* stack_frame = pmm_alloc_frame(stack_size >> 12);
    ASSERT(stack_frame);
    u64 stack_address = 0x7ff000000000;

    Region stack_region = region_create(stack_address, stack_size, false, true, -1);
    regionlist_append(&region_list, stack_region);
    region_map(stack_region, page_table_address, (u64)stack_frame);

    ksp("Stack: %lx Entry: %lx CR3: %lx\n", 
        stack_address,
        elf.header.e_entry,
        to_lower_half(page_table_address));

    page_table_active_walk_and_print(stack_address, page_table_address);
    bochs_breakpoint();
    set_page_table_and_jump(to_lower_half(page_table_address), stack_address, elf.header.e_entry);

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
