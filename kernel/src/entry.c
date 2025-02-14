#include "stdint.h"
#include <stddef.h>
#include <stdbool.h>

#include "./kio.h"
#include "./idt.h"
#include "./io.h"
#include "stdint.h"
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
#include "task.c"
#include "scheduler.c"



extern u8 _KERNEL_TXT_START;
extern u8 _KERNEL_TXT_END;

#define KERNEL_STACK_SIZE 16384
static u8 kernel_stack[KERNEL_STACK_SIZE] __attribute__((aligned(8)));
static u64 kernel_stack_ptr = (u64) &kernel_stack + KERNEL_STACK_SIZE;

// The following will be our kernel's entry point.
// If renaming _start() to something else, make sure to change the
// linker script accordingly.
void _start(void)
{
    __asm__ volatile ("mov %0, %%rsp" : : "a" (kernel_stack_ptr));
    // Ensure the bootloader actually understands our base revision (see spec).
    context_init();

    gdt_init(kernel_stack_ptr);
    init_idt();
    bochs_breakpoint();

    // Fetch the first framebuffer.
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    volatile u32 *fb_ptr = framebuffer->address;

    // Note: we assume the framebuffer model is RGB with 32-bit pixels.
    for (size_t i = 0; i < 100; i++)
    {
        fb_ptr[i * (framebuffer->pitch / 4) + i] = 0xffffff;
    }

    PmmAllocator pmm_allocator = pmm_init(memmap_request, hhdm_request, kernel_address_request);
    ASSERT_EQ((u32)module_request.response->module_count, 1); // make sure that we have atleast one file.
    (void)pmm_allocator;


    /*
    for (;;) {
        Frame frame = pmm_alloc_frame(&pmm_allocator, 1);
        // ksp("0x%lb ", allocator.bmp[0]);
        // ksp("0x%lb ", allocator.bmp[1]);
        // ksp("0x%lb [%d]\n", allocator.bmp[2], i);
        ASSERT(frame.ptr);
    }

    for (int i = 0; i < 192; i++) {
        Frame frame = frame_create(i << 12);
        pmm_dealloc_frame(&allocator, frame, 1);
        // ksp("0x%lb ", allocator.bmp[0]);
        // ksp("0x%lb ", allocator.bmp[1]);
        // ksp("0x%lb [%d]\n", allocator.bmp[2], i);
    }
    */



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

    u64 current_page_table_address = to_higher_half(vmm_cr3());

    Elf64 program_elf = elf_parse(module_request.response->modules[0]->address, module_request.response->modules[0]->size);

    scheduler_init();

    // TODO: 450+ causes general protection fault here somehow.
    for (int i = 0; i < 425; i++) { 
        s64 argc = 3;
        char* argv[] = {"hello-world", "hello darkness", "15"};
        Task task = task_init(&pmm_allocator, (PageTableEntry*) current_page_table_address, program_elf, argc, argv);
        scheduler_queue_task(task);
    }

    scheduler_idle_loop();

    while(1) {}

}
