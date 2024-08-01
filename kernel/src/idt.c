#ifndef IDT_H
#define IDT_H

#include <stdint.h>
#include "./kio.h"
#include "./pic.c"

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
   uint64_t interrupt_number;
   // uint64_t rip;

   // uint16_t cs;
   // uint16_t ds;
   // uint16_t ss;
   // uint16_t fs;
   // uint16_t gs;
};

void dmpregs(struct regs r) {
   ksp("rax     %lx\n", r.rax);   
   ksp("rbx     %lx\n", r.rbx);   
   ksp("rcx     %lx\n", r.rcx);   
   ksp("rdx     %lx\n", r.rdx);   
   ksp("rbp     %lx\n", r.rbp);   
   ksp("rsi     %lx\n", r.rsi);   
   ksp("rdi     %lx\n", r.rdi);   
   ksp("rsp     %lx\n", r.rsp);   
   ksp("r8      %lx\n", r.r8);
   ksp("r9      %lx\n", r.r9);
   ksp("r10     %lx\n", r.r10);   
   ksp("r11     %lx\n", r.r11);   
   ksp("r12     %lx\n", r.r12);   
   ksp("r13     %lx\n", r.r13);   
   ksp("r14     %lx\n", r.r14);   
   ksp("r15     %lx\n", r.r15);   
   ksp("rflags  %lx\n", r.rflags);
}
/* Interrupt and Exceptions */
static void halt() { while(1) {} }

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

//static int i = 0;

struct regs timer(struct regs r) { 
   ksp("timer!\n");
   pic_send_end_of_interrupt();
   return r;
} // 21

void all_interrupts_handler(struct regs r)
{
   switch (r.interrupt_number) {
      case 32:
         timer(r);
         break;
      default:
         ksp("we received an generic interrupt %ld\n", r.interrupt_number);
         dmpregs(r);
         while(1) {}
   };
}

// __attribute__ ((interrupt))
// static void generic_exception_handler(struct interrupt_frame *frame, uint64_t error_code)
// {
//    (void)frame;
//    (void)error_code;
//    ksp("we received an generic exception\n");
//    while (1) {}
// }

extern void int_wrapper_0();
extern void int_wrapper_1();
extern void int_wrapper_2();
extern void int_wrapper_3();
extern void int_wrapper_4();
extern void int_wrapper_5();
extern void int_wrapper_6();
extern void int_wrapper_7();
extern void int_wrapper_8();
extern void int_wrapper_9();
extern void int_wrapper_10();
extern void int_wrapper_11();
extern void int_wrapper_12();
extern void int_wrapper_13();
extern void int_wrapper_14();
extern void int_wrapper_15();
extern void int_wrapper_16();
extern void int_wrapper_17();
extern void int_wrapper_18();
extern void int_wrapper_19();
extern void int_wrapper_20();
extern void int_wrapper_21();

// hardware interrupts
extern __attribute__((interrupt)) void int_wrapper_32(struct interrupt_frame*, uint64_t);
extern __attribute__((interrupt)) void int_wrapper_33(struct interrupt_frame*, uint64_t);


void divide_error                 (struct interrupt_frame* iframe             ) { (void)iframe; ksp("divide error");                  halt(); } // 0
void debug                        (struct interrupt_frame* iframe             ) { (void)iframe; ksp("debug");                         halt(); } // 1
void nmi_interrupt                (struct interrupt_frame* iframe             ) { (void)iframe; ksp("nmi interrupt");                 halt(); } // 2
void breakpoint                   (struct interrupt_frame* iframe             ) { (void)iframe; ksp("breakpoint");                    halt(); } // 3
void overflow                     (struct interrupt_frame* iframe             ) { (void)iframe; ksp("overflow error");                halt(); } // 4
void bound_range_exceeded         (struct interrupt_frame* iframe             ) { (void)iframe; ksp("bound range exceeded");          halt(); } // 5
void invalid_opcode               (struct interrupt_frame* iframe             ) { (void)iframe; ksp("invalid opcode");                halt(); } // 6
void device_not_available         (struct interrupt_frame* iframe             ) { (void)iframe; ksp("device not available");          halt(); } // 7
void double_fault                 (struct interrupt_frame* iframe, uint64_t ui) { (void)iframe; (void)ui; ksp("EXCEPTION: double fault");       halt(); } // 8
void co_processor_segment_overrun (struct interrupt_frame* iframe             ) { (void)iframe; ksp("co-processor segment overrun");  halt(); } // 9
void invalid_tss                  (struct interrupt_frame* iframe, uint64_t ui) { (void)iframe; (void)ui; ksp("EXCEPTION: invalid tss");        halt(); } // 10
void segment_not_present          (struct interrupt_frame* iframe, uint64_t ui) { (void)iframe; (void)ui; ksp("EXCEPTION: segment not present");halt(); } // 11
void stack_segment_fault          (struct interrupt_frame* iframe, uint64_t ui) { (void)iframe; (void)ui; ksp("EXCEPTION: stack segment fault");halt(); } // 12
void general_protection           (struct interrupt_frame* iframe, uint64_t ui) { (void)iframe; (void)ui; ksp("EXCEPTION: general protection"); halt(); } // 13
void page_fault                   (struct interrupt_frame* iframe, uint64_t ui) { (void)iframe; (void)ui; ksp("EXCEPTION: page fault");         halt(); } // 14
void floating_point_error         (struct interrupt_frame* iframe             ) { (void)iframe; ksp("floating point error");          halt(); } // 16
void alignment_check              (struct interrupt_frame* iframe, uint64_t ui) { (void)iframe; (void)ui; ksp("EXCEPTION: alignment check");    halt(); } // 17
void machine_check                (struct interrupt_frame* iframe             ) { (void)iframe; ksp("machine_check");                 halt(); } // 18
void simd_floating_point_exception(struct interrupt_frame* iframe             ) { (void)iframe; ksp("simd floating point exception"); halt(); } // 19
void virtualization_exception     (struct interrupt_frame* iframe             ) { (void)iframe; ksp("virtualization exception");      halt(); } // 20
void control_protection_exception (struct interrupt_frame* iframe             ) { (void)iframe; ksp("control protection exception");  halt(); } // 21


void idt_set_handler(int interrupt_vector, void* handler_fn, uint8_t type_attribute) {
   idt_entry_64_t * entry = &idt[interrupt_vector];

   entry->offset_1        = ((uint64_t)handler_fn & 0xffff);
   entry->selector        = 0x28;                                       // offset of the GDT kernel code segment
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

   idt_set_handler(0,  int_wrapper_0,  0x8E);
   idt_set_handler(1,  int_wrapper_1,  0x8E);
   idt_set_handler(2,  int_wrapper_2,  0x8E);
   idt_set_handler(3,  int_wrapper_3,  0x8E);
   idt_set_handler(4,  int_wrapper_4,  0x8E);
   idt_set_handler(5,  int_wrapper_5,  0x8E);
   idt_set_handler(6,  int_wrapper_6,  0x8E);
   idt_set_handler(7,  int_wrapper_7,  0x8E);
   idt_set_handler(8,  int_wrapper_8,  0x8F); //
   idt_set_handler(9,  int_wrapper_9,  0x8E);
   idt_set_handler(10, int_wrapper_10, 0x8F); //
   idt_set_handler(11, int_wrapper_11, 0x8F); //
   idt_set_handler(12, int_wrapper_12, 0x8F); //
   idt_set_handler(13, int_wrapper_13, 0x8F); //
   idt_set_handler(14, int_wrapper_14, 0x8F); //
   idt_set_handler(16, int_wrapper_16, 0x8E);
   idt_set_handler(17, int_wrapper_17, 0x8F); //
   idt_set_handler(18, int_wrapper_18, 0x8E);
   idt_set_handler(19, int_wrapper_19, 0x8E);
   idt_set_handler(20, int_wrapper_20, 0x8E);
   idt_set_handler(21, int_wrapper_21, 0x8E);

    asm volatile ("xchgw %bx, %bx");
   __asm__ volatile("lidt %0" :: "m"(idtr));          // load the new IDT 

 int flags = 0;
   init_pic();
   __asm__ volatile("sti");                           // set the interrupt flag
  
   __asm__ volatile("pushf\n\t"
                 "pop %%rax\n\t"
                 "and $0x200, %%rax"
                 : "=a" (flags));

   if (flags) {
    ksp("Interrupts are enabled\n");
} else {
    ksp("Interrupts are still disabled\n");
}
   
}

void init_pic(void) {
   // we need to offset the pic interrupts, as they will overlap over the 
   // software interrupts we defined above.
   pic_remap(0x20, 0x28);

   // setup hardware interrupt handlers
   idt_set_handler(0x20, int_wrapper_32, 0x8E);
   idt_set_handler(0x21, int_wrapper_33, 0x8E);
   for (int vector = 0x22; vector < 0x29; vector++) {
      idt_set_handler(vector, all_interrupts_handler, 0x8E);
   }

   // disable/mask all the hardware interrupts right now.
   // until we implement keyboard drivers.
   // pic_mask_all_interrupts();
}

#endif
