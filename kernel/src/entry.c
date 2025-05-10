#include "./defs.c"

#include "stdint.h"
#include <stddef.h>
#include <stdbool.h>

#include "./kio.h"
#include "stdint.h"
#include "bochs.h"
#include "asa_limine.h"
#include "cpu.h"


#include "tinyubsan.c"
#include "./io.c"
#include "./idt.c"
#include "./pic.c"
#include "./kio.c"
#include "./kstring.c"
#include "pmm.c"
#include "vmm.c"
#include "elf.c"
#include "gdt.c"
#include "syscall.c"
#include "task.c"
#include "scheduler.c"


extern u8 _KERNEL_TXT_START;
extern u8 _KERNEL_TXT_END;

#define KERNEL_STACK_SIZE 16384
static u8 kernel_stack[KERNEL_STACK_SIZE] __attribute__((aligned(8)));
static u64 kernel_stack_ptr = (u64) &kernel_stack + KERNEL_STACK_SIZE;

#define INTERRUPT_STACK_SIZE 16384
static u8 interrupt_stack[INTERRUPT_STACK_SIZE] __attribute__((aligned(8)));
static u64 interrupt_stack_ptr = (u64) &interrupt_stack + INTERRUPT_STACK_SIZE;


typedef struct ProcessorContext {
    u64 user_stack_ptr;
    u64 kernel_stack_ptr;
} ProcessorContext;


ProcessorContext context = {0};

// The following will be our kernel's entry point.
// If renaming _start() to something else, make sure to change the
// linker script accordingly.
void _start(void)
{
    __asm__ volatile ("mov %0, %%rsp" : : "a" (kernel_stack_ptr));
    context.kernel_stack_ptr = kernel_stack_ptr;
    // Ensure the bootloader actually understands our base revision (see spec).
    context_init();

    gdt_init(kernel_stack_ptr, interrupt_stack_ptr);
    init_idt();

    // Fetch the first framebuffer.
    struct limine_framebuffer *framebuffer = ctx_get_framebuffer();
    volatile u32 *fb_ptr = framebuffer->address;

    // Note: we assume the framebuffer model is RGB with 32-bit pixels.
    for (size_t i = 0; i < 100; i++)
    {
        fb_ptr[i * (framebuffer->pitch / 4) + i] = 0xffffff;
    }

    PmmAllocator pmm_allocator = pmm_init(MEMMAP_REQUEST, HHDM_REQUEST, KERNEL_ADDRESS_REQUEST);
    (void)pmm_allocator;

    // for syscalls:
    // IF (CS.L ≠ 1 ) or (IA32_EFER.LMA ≠ 1) or (IA32_EFER.SCE ≠ 1)
    // Things we need to add to the MSR:
    // 1. RIP := IA32_LSTAR;
    // 2. CS.Selector := IA32_STAR[47:32] AND FFFCH
    // 
    wrmsr(CPU_IA32_EFER, rdmsr(CPU_IA32_EFER) | (1UL << 0)); // we are enabling fast syscall in the processor.
    wrmsr(CPU_IA32_FSTAR, 0x43700); // Clear IF,TF,AC, and DF
    wrmsr(CPU_IA32_LSTAR, (u64) &syscall_handler_wrapper);    // this is syscall entry function, currently hang function
    wrmsr(CPU_IA32_STAR, 0x0030002800000000);

    wrmsr(CPU_IA32_KERNEL_GS_BASE, (u64) &context);
    wrmsr(CPU_IA32_USER_GS_BASE, (u64) &context);

    u64 current_page_table_address = to_higher_half(vmm_cr3());

    struct limine_file ** modules = ctx_get_modules();
    Elf64 program_elf = elf_parse(modules[0]->address, modules[0]->size);

    scheduler_init();

    for (int i = 0; i < 100; i++) { 
        u64 argc = 3;
        char* argv[] = {"hello-world", "hello darkness", "1000"};
        Task task = task_init(&pmm_allocator, (PageTableEntry*) current_page_table_address, program_elf, argc, argv);
        scheduler_queue_task(task);
    }

    scheduler_idle_loop();

    while(1) {}

}
