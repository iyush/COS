#ifndef IDT_H
#define IDT_H

#include <stdint.h>
#include "./kio.h"
#include "./pic.h"
#include "./idt.h"

typedef struct {
   uint16_t offset_1;        // offset bits 0..15
   uint16_t selector;        // a code segment selector in GDT or LDT
   uint8_t  ist;             // bits 0..2 holds Interrupt Stack Table offset, rest of bits zero.
   uint8_t  type_attributes; // gate type, dpl, and p fields
   uint16_t offset_2;        // offset bits 16..31
   uint32_t offset_3;        // offset bits 32..63
   uint32_t zero;            // reserved
} __attribute__((packed)) idt_entry_64_t;

typedef struct {
	uint16_t	limit;
	uint64_t	base;
} __attribute__((packed)) idtr_t;

struct __attribute__((packed)) regs {
   uint64_t rax;
   uint64_t rbx;
   uint64_t rcx;
   uint64_t rdx;
   uint64_t rbp;
   uint64_t rsi;
   uint64_t rdi;
   uint64_t rsp;
   uint64_t r8;
   uint64_t r9;
   uint64_t r10;
   uint64_t r11;
   uint64_t r12;
   uint64_t r13;
   uint64_t r14;
   uint64_t r15;

   uint64_t rflags;
   uint64_t rip;

   uint16_t cs;
   uint16_t ds;
   uint16_t ss;
   uint16_t fs;
   uint16_t gs;
};

static idt_entry_64_t idt[256];

static idtr_t idtr;

struct interrupt_frame
{
    uint64_t eip;
    uint64_t cs;
    uint64_t flags;
    uint64_t sp;
    uint64_t ss;
};

__attribute__ ((interrupt))
static void generic_interrupt_handler(struct interrupt_frame *frame)
{
   (void)frame;
   ksp("we received an generic interrupt");
   while (1) {}
}

__attribute__ ((interrupt))
static void generic_exception_handler(struct interrupt_frame *frame, uint64_t error_code)
{
   (void)frame;
   (void)error_code;
   ksp("we received an generic exception \n");
   while (1) {}
}

extern void int_wrapper_1();
extern void int_wrapper_2();
extern void int_wrapper_3();
extern void int_wrapper_4();
extern void int_wrapper_5();
extern void int_wrapper_6();
extern void int_wrapper_7();
extern void int_wrapper_8();
extern void int_wrapper_9();

/* Interrupt and Exceptions */
static void halt() { while(1) {} }

static void divide_error                 (struct interrupt_frame*          ) { ksp("divide error");                  halt(); } // 0
static void debug                        (struct interrupt_frame*          ) { ksp("debug");                         halt(); } // 1
static void nmi_interrupt                (struct interrupt_frame*          ) { ksp("nmi interrupt");                 halt(); } // 2
static void breakpoint                   (struct interrupt_frame*          ) { ksp("breakpoint");                    halt(); } // 3
static void overflow                     (struct interrupt_frame*          ) { ksp("overflow error");                halt(); } // 4
static void bound_range_exceeded         (struct interrupt_frame*          ) { ksp("bound range exceeded");          halt(); } // 5
static void invalid_opcode               (struct interrupt_frame*          ) { ksp("invalid opcode");                halt(); } // 6
static void device_not_available         (struct interrupt_frame*          ) { ksp("device not available");          halt(); } // 7
static void double_fault                 (struct interrupt_frame*, uint64_t) { ksp("EXCEPTION: double fault");       halt(); } // 8
static void co_processor_segment_overrun (struct interrupt_frame*          ) { ksp("co-processor segment overrun");  halt(); } // 9
static void invalid_tss                  (struct interrupt_frame*, uint64_t) { ksp("EXCEPTION: invalid tss");        halt(); } // 10
static void segment_not_present          (struct interrupt_frame*, uint64_t) { ksp("EXCEPTION: segment not present");halt(); } // 11
static void stack_segment_fault          (struct interrupt_frame*, uint64_t) { ksp("EXCEPTION: stack segment fault");halt(); } // 12
static void general_protection           (struct interrupt_frame*, uint64_t) { ksp("EXCEPTION: general protection"); halt(); } // 13
static void page_fault                   (struct interrupt_frame*, uint64_t) { ksp("EXCEPTION: page fault");         halt(); } // 14
static void floating_point_error         (struct interrupt_frame*          ) { ksp("floating point error");          halt(); } // 16
static void alignment_check              (struct interrupt_frame*, uint64_t) { ksp("EXCEPTION: alignment check");    halt(); } // 17
static void machine_check                (struct interrupt_frame*          ) { ksp("machine_check");                 halt(); } // 18
static void simd_floating_point_exception(struct interrupt_frame*          ) { ksp("simd floating point exception"); halt(); } // 19
static void virtualization_exception     (struct interrupt_frame*          ) { ksp("virtualization exception");      halt(); } // 20
static void control_protection_exception (struct interrupt_frame*          ) { ksp("control protection exception");  halt(); } // 21

void idt_set_handler(int interrupt_vector, void* handler_fn, uint8_t type_attribute) {
   idt_entry_64_t * entry = &idt[interrupt_vector];

   entry->offset_1        = ((uint64_t)handler_fn & 0xffff);
   entry->selector        = 8;                                       // offset of the GDT kernel code segment
   entry->ist             = 0;
   entry->type_attributes = type_attribute;
   entry->offset_2        = ((uint64_t)handler_fn >> 16) & 0xffff;
   entry->offset_3        = (uint32_t)((uint64_t)handler_fn >> 32) & 0xffffffff;
   entry->zero            = 0;
}

void init_pic(void);

void init_idt(void) {
   // setup the interrupt descriptor table
   idtr.limit = (uint16_t)sizeof(idt_entry_64_t) * 256;
   idtr.base = (uint64_t)&idt[0];

   for (int vector = 0; vector < 32; vector++) {
      if (vector == 8 || vector == 10 || vector == 11 || vector == 12 
            || vector == 13 || vector == 14 || vector == 17) {
         idt_set_handler(vector, generic_exception_handler, 0x8F);
      } else {
         idt_set_handler(vector, generic_interrupt_handler, 0x8E);
      }
   }

   idt_set_handler(0,  int_wrapper_1                , 0x8E);
   idt_set_handler(1,  int_wrapper_2                        , 0x8E);
   idt_set_handler(2,  nmi_interrupt                , 0x8E);
   idt_set_handler(3,  breakpoint                   , 0x8E);
   idt_set_handler(4,  overflow                     , 0x8E);
   idt_set_handler(5,  bound_range_exceeded         , 0x8E);
   idt_set_handler(6,  invalid_opcode               , 0x8E);
   idt_set_handler(7,  device_not_available         , 0x8E);
   idt_set_handler(8,  double_fault                 , 0x8F); //
   idt_set_handler(9,  co_processor_segment_overrun , 0x8E);
   idt_set_handler(10, invalid_tss                  , 0x8F); //
   idt_set_handler(11, segment_not_present          , 0x8F); //
   idt_set_handler(12, stack_segment_fault          , 0x8F); //
   idt_set_handler(13, general_protection           , 0x8F); //
   idt_set_handler(14, page_fault                   , 0x8F); //
   idt_set_handler(16, floating_point_error         , 0x8E);
   idt_set_handler(17, alignment_check              , 0x8F); //
   idt_set_handler(18, machine_check                , 0x8E);
   idt_set_handler(19, simd_floating_point_exception, 0x8E);
   idt_set_handler(20, virtualization_exception     , 0x8E);
   idt_set_handler(21, control_protection_exception , 0x8E);


   __asm__ volatile("lidt %0" :: "m"(idtr));          // load the new IDT 
                                                      

   init_pic();
   __asm__ volatile("sti");                           // set the interrupt flag
}

void init_pic(void) {
   // we need to offset the pic interrupts, as they will overlap over the 
   // software interrupts we defined above.
   pic_remap(0x20, 0x28);

   // setup hardware interrupt handlers
   for (int vector = 0x20; vector < 0x29; vector++) {
      idt_set_handler(vector, generic_interrupt_handler, 0x8E);
   }

   // disable/mask all the hardware interrupts right now.
   // until we implement keyboard drivers.
    pic_mask_all_interrupts();
}

#endif
