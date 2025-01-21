#include "stdint.h"
#include <stddef.h>
#include <stdbool.h>

#include "./kio.h"
#include "./idt.h"
#include "./io.h"
#include "stdint.h"
#include <pmm.h>
#include "bochs.h"
#include "asa_limine.h"
#include "gdt.h"
#include "cpu.h"


#include "./idt.c"
#include "./pic.c"
#include "./kio.c"
#include "./io.c"
#include "./kstring.c"
#include "pmm.c"
#include "vmm.c"
#include "elf.c"
#include "gdt.c"


#define MAX_REGION_LIST_TASK 1024

extern u8 _KERNEL_TXT_START;
extern u8 _KERNEL_TXT_END;

#define KERNEL_STACK_SIZE 16384
static u8 kernel_stack[KERNEL_STACK_SIZE] __attribute__((aligned(8)));
static u64 kernel_stack_ptr = (u64) &kernel_stack + KERNEL_STACK_SIZE;


// Halt and catch fire function.
static void hcf(void)
{
    asm("cli");
    for (;;)
    {
        asm("hlt");
    }
}

void set_page_table_and_jump(u64 page_table_frame, u64 stack_address, u64 entry_point, u64 return_address) {
    bochs_breakpoint();
    __asm__ volatile(
        "\n mov %0, %%cr3"                  // load the page table
        "\n mov %1, %%rsp"                  // change the stack pointer
        "\n pushq %2"                       // push the return address

        // now are setting up the iretq for usermode executable
        "\n mov %%rsp, %%rax"               // save the current stack ptr
        "\n pushq $0x40 | 3"                // stack segment (ss)
        "\n pushq %%rax"                    // rsp (this is the stack address that we saved earlier)
        "\n pushq $0x202"                   // rflags
        "\n pushq $0x38 | 3"                // code segment (cs)
        "\n pushq %3"                       // jump destination
        "\n iretq"
        :
        : "r"(page_table_frame), "r"(stack_address), "r"(return_address), "r"(entry_point)
        : "rdi", "rax"
        );
}


// The following will be our kernel's entry point.
// If renaming _start() to something else, make sure to change the
// linker script accordingly.
void _start(void)
{
    __asm__ volatile ("mov %0, %%rsp" : : "a" (kernel_stack_ptr));
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
    gdt_init(kernel_stack_ptr);
    init_idt();

    // Fetch the first framebuffer.
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    volatile u32 *fb_ptr = framebuffer->address;

    // Note: we assume the framebuffer model is RGB with 32-bit pixels.
    for (size_t i = 0; i < 100; i++)
    {
        fb_ptr[i * (framebuffer->pitch / 4) + i] = 0xffffff;
    }

    PmmAllocator pmm_allocator = pmm_init(memmap_request, hhdm_request, kernel_address_request);
    vmm_init(&pmm_allocator, hhdm_request);

    ASSERT_EQ((u32)module_request.response->module_count, 1); // make sure that we have atleast one file.

    u64 current_page_table_address = to_higher_half(vmm_cr3());
    u64 page_table_address = page_table_create();
    RegionList region_list = regionlist_create(&pmm_allocator, MAX_REGION_LIST_TASK);


    { // parsing kernel elf
        void* kernel_address = kfile_request.response->kernel_file->address;
        u64 kernel_size = (u64)kfile_request.response->kernel_file->size;
        Elf64 kernel_elf = elf_parse(kernel_address, kernel_size);
        ASSERT(kernel_elf.p_headers_len > 0);
        ASSERT(kernel_elf.s_headers_len > 0);

        for (u64 i = 0; i < kernel_elf.p_headers_len; i++) {
            Elf64_Phdr pheader = kernel_elf.p_headers[i];
            (void) page_table_address;
            if (pheader.p_type == PT_LOAD) {
                pheader.p_vaddr = align_down(pheader.p_vaddr);
                pheader.p_memsz = align_up(pheader.p_memsz);

                ASSERT(pheader.p_vaddr % 0x1000 == 0);

                u64 physical_frame = vmm_physical_frame(current_page_table_address, pheader.p_vaddr);

                // reserve the virtual address;
                Region region = region_create(pheader.p_vaddr, pheader.p_memsz);

                u64 flags = FRAME_PRESENT;// | FRAME_USER;

                // if (!(pheader.p_flags & PF_X)) flags |= FRAME_NOEXEC;
                if (pheader.p_flags & PF_W) flags |= FRAME_WRITABLE;

                regionlist_append(&region_list, region);
                region_map(region, page_table_address, physical_frame, flags);
            }
        }
    }


    void* elf_module_start = module_request.response->modules[0]->address; 
    u64 elf_module_size = module_request.response->modules[0]->size; 
    Elf64 program_elf = elf_parse(elf_module_start, elf_module_size);
    
    for (u64 i = 0; i < program_elf.p_headers_len; i++) {
        Elf64_Phdr pheader = program_elf.p_headers[i];
        if (pheader.p_type == PT_LOAD) {
            pheader.p_vaddr = align_down(pheader.p_vaddr);
            pheader.p_memsz = align_up(pheader.p_memsz);
            pheader.p_offset = align_down(pheader.p_offset);

            ASSERT(pheader.p_vaddr % 0x1000 == 0);
            ASSERT(pheader.p_offset % 0x1000 == 0);

            // reserve the virtual address;
            Region region = region_create(pheader.p_vaddr, pheader.p_memsz);

            u64 flags = FRAME_PRESENT | FRAME_USER;

            // if (pheader.p_flags & PF_R) region.is_writable = false;
            if (pheader.p_flags & PF_W) flags |= FRAME_WRITABLE; // region.is_writable = true;

            regionlist_append(&region_list, region);
            region_map(region, page_table_address, to_lower_half(pheader.p_offset + (u64)elf_module_start), flags);
        }
    }

    // IMPORTANT: map the kernel on higher half.
    // u64 kernel_text_size = align_up((u64)&_KERNEL_END - (u64)&_KERNEL_START);
    // Region kernel_region = region_create(kernel_address_request.response->virtual_base, kernel_text_size);
    // regionlist_append(&region_list, kernel_region);
    // region_map(kernel_region, page_table_address, kernel_address_request.response->physical_base, FRAME_PRESENT | FRAME_USER);

    // IMPORTANT: create a stack for the executable
    u64 stack_size = 0x100000; // 1Mib;
    void* stack_frame = pmm_alloc_frame(&pmm_allocator, stack_size >> 12);
    ASSERT(stack_frame);
    u64 stack_address = 0x7ff000000000;

    Region stack_region = region_create(stack_address, stack_size);
    regionlist_append(&region_list, stack_region);
    region_map(stack_region, page_table_address, (u64)stack_frame, FRAME_PRESENT | FRAME_WRITABLE | FRAME_USER);

    // for syscalls:
    // IF (CS.L ≠ 1 ) or (IA32_EFER.LMA ≠ 1) or (IA32_EFER.SCE ≠ 1)
    // Things we need to add to the MSR:
    // 1. RIP := IA32_LSTAR;
    // 2. CS.Selector := IA32_STAR[47:32] AND FFFCH
    // 

    wrmsr(CPU_IA32_EFER, rdmsr(CPU_IA32_EFER) | (1UL << 0)); // we are enabling fast syscall in the processor.
    wrmsr(CPU_IA32_FSTAR, 0x43700); // Clear IF,TF,AC, and DF
    wrmsr(CPU_IA32_LSTAR, (u64) &int_wrapper_99);    // this is syscall entry function, currently hang function
    wrmsr(CPU_IA32_STAR, rdmsr(CPU_IA32_STAR) | (0x28UL << 32) | (0x38UL << 48)); // this is our CS for kernel.

    set_page_table_and_jump(to_lower_half(page_table_address), stack_address + stack_size, program_elf.header.e_entry, (u64) &int_wrapper_99);
    while(1) {}
}
