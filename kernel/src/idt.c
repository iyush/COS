#ifndef IDT_H
#define IDT_H

#include "stdint.h"
#include "./kio.h"
#include "./pic.c"
#include "idt.h"

typedef struct {
   u16 offset_1;        // offset bits 0..15
   u16 selector;        // a code segment selector in GDT or LDT
   u8  ist;             // bits 0..2 holds Interrupt Stack Table offset, rest of bits zero.
   u8  type_attributes; // gate type, dpl, and p fields
   u16 offset_2;        // offset bits 16..31
   u32 offset_3;        // offset bits 32..63
   u32 zero;            // reserved
} __attribute__((packed)) idt_entry_64_t;

typedef struct {
	u16	limit;
	u64	base;
} __attribute__((packed)) idtr_t;


void hang() {

   while (1) {
      __asm__ volatile("hlt");
      
   }

}
void dmpregs(struct regs r) {
   ksp("rax     0x%lx\n", r.rax);
   ksp("rbx     0x%lx\n", r.rbx);
   ksp("rcx     0x%lx\n", r.rcx);
   ksp("rdx     0x%lx\n", r.rdx);
   ksp("rbp     0x%lx\n", r.rbp);
   ksp("rsi     0x%lx\n", r.rsi);
   ksp("rdi     0x%lx\n", r.rdi);
   ksp("rsp     0x%lx\n", r.rsp);
   ksp("r8      0x%lx\n", r.r8);
   ksp("r9      0x%lx\n", r.r9);
   ksp("r10     0x%lx\n", r.r10);
   ksp("r11     0x%lx\n", r.r11);
   ksp("r12     0x%lx\n", r.r12);
   ksp("r13     0x%lx\n", r.r13);
   ksp("r14     0x%lx\n", r.r14);
   ksp("r15     0x%lx\n", r.r15);
   ksp("rflags  0x%lx\n", r.rflags);
   ksp("error_code  0x%lx\n", r.error_code);
   ksp("rip  0x%lx\n", r.rip);
}

static idt_entry_64_t idt[256];

static idtr_t idtr;

struct interrupt_frame
{
    u64 eip;
    u64 cs;
    u64 flags;
    u64 sp;
    u64 ss;
};

void timer(struct regs* r) {
   (void)r;
   pic_send_end_of_interrupt();
} // 21

void all_interrupts_handler(struct regs* r)
{
   switch (r->interrupt_number) {
      case 13:
      {
         ksp("We got a General Protection Fault!\n");
         dmpregs(*r);
         while(1) {}
      } break;
      case 14:
      {
         ksp("We got a page fault!\n");
         u64 cr2 = 0;
         asm volatile("mov %%cr2, %0" : "=r" (cr2));
         ksp("Page fault happend at address: 0x%lx\n", cr2);
         dmpregs(*r);
         while(1){ }
      } break;
      case 32:
         // ksp("Timer timer!\n");
         timer(r);
         break;
      case 33:
         //TODO: keyboard
         break;
      case 99: {
                  switch(r->rax) {
                     case 0: 
                        {
                           hang();
                        }
                        break;
                     case 1:
                        {
                           output_to_console((char*)r->rdi, r->rsi);
                        }
                        break;
                  };
               }
         break;
         

      default:
         ksp("we received an generic interrupt %ld\n", r->interrupt_number);
         dmpregs(*r);
         while(1) {}
   };
}

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
extern __attribute__((interrupt)) void int_wrapper_32(struct interrupt_frame*, u64);
extern __attribute__((interrupt)) void int_wrapper_33(struct interrupt_frame*, u64);


// syscall
extern __attribute__((interrupt)) void int_wrapper_99(struct interrupt_frame*, u64);


void idt_set_handler(int interrupt_vector, void* handler_fn, u8 type_attribute) {
   idt_entry_64_t * entry = &idt[interrupt_vector];

   entry->offset_1        = ((u64)handler_fn & 0xffff);
   entry->selector        = 0x28;                                       // offset of the GDT kernel code segment
   entry->ist             = 0;
   entry->type_attributes = type_attribute;
   entry->offset_2        = ((u64)handler_fn >> 16) & 0xffff;
   entry->offset_3        = (u32)((u64)handler_fn >> 32) & 0xffffffff;
   entry->zero            = 0;
}

void init_pic(void);

void init_idt(void) {
   // setup the interrupt descriptor table
   idtr.limit = (u16)sizeof(idt_entry_64_t) * 256;
   idtr.base = (u64)&idt[0];

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
   // pic_disable_all_interrupts(); // here masking means disabling
}

#endif
