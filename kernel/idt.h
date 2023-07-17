#include <stdint.h>
#include "./kio.h"


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

void idt_set_handler(int interrupt_vector, void* handler_fn, uint8_t type_attribute) {
   idt_entry_64_t * entry = &idt[interrupt_vector];

   entry->offset_1        = ((uint64_t)handler_fn & 0xffff);
   entry->selector        = 8;                                       // 0x8 if it is 32 bit and 0x10 as it is 64 bit?
   entry->ist             = 0;
   entry->type_attributes = type_attribute;
   entry->offset_2        = ((uint64_t)handler_fn >> 16) & 0xffff;
   entry->offset_3        = (uint32_t)((uint64_t)handler_fn >> 32) & 0xffffffff;
   entry->zero            = 0;
}

void init_idt(void) {
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

   __asm__ volatile("lidt %0" :: "m"(idtr));          // load the new IDT 
   __asm__ volatile("sti");                           // set the interrupt flag
}

