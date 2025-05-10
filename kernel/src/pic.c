#ifndef PIC_H
#define PIC_H
#include "./kio.h"

#define PIC_MASTER_COMMAND   0x0020
#define PIC_MASTER_DATA      0x0021
#define PIC_SLAVE_COMMAND    0x00A0
#define PIC_SLAVE_DATA       0x00A1

#define ICW1_IC4             (1<<0) // if this is set - icw4 has to be read. if icw4 is not needed
                                    // set ic4=0

#define ICW1_SNGL            (1<<1) // Means that this is the only 8259A in the system. If SNGL = 1,
                                    // no ICW3 will be issued. We have 2 PICs in the system.

#define ICW1_ADI             (1<<2) // Call address interval, if ADI = 1, then interval = 4, if ADI == 0,
                                    // then interval = 8. This is usually ignored by x86.
                                  
#define ICW1_LTIM            (1<<3) // If LTIM = 1, then the 8259A will operate in
                                    // the level interrupt mode. Edge detect logic
                                    // on the interrupt inputs will be disabled. By default we are in 
                                    // edge triggered mode, so dont use this.

#define ICW1_INIT            (1<<4) // Initialization

#define ICW4_UPM	           (1<<0) // if 1, it is in 80x86 mode, else it is in MCS-80/85 mode

#define ICW4_AEOI	           (1<<1)	// if 1, on the last interrupt acknowledge signal, 
                                    // PIC automatically performs End of Interrupt (EOI) operation

#define ICW4_MS	             (1<<2)	// Only use if ICW4_BUF is set. If 1, selects buffer master. if 0, buffer slave.

#define ICW4_BUF	           (1<<3)	// If 1, controller operates in buffered mode.

#define ICW4_SFNM	           (1<<4)	// Special Fully Nested Mode. Used in systems with a large amount of cascaded controllers.
                                  
void pic_remap(s8 master_offset, s8 slave_offset) {
   // save the current state or "masks"
   s8 master_data = inb(PIC_MASTER_DATA);
   s8 slave_data = inb(PIC_SLAVE_DATA);

   // start the initialization with cascade mode.
   outb(PIC_MASTER_COMMAND, ICW1_INIT | ICW1_IC4); io_wait();
   outb(PIC_SLAVE_COMMAND , ICW1_INIT | ICW1_IC4); io_wait();

   // master and slave pic vector offset
   outb(PIC_MASTER_DATA, master_offset); io_wait();
   outb(PIC_SLAVE_DATA , slave_offset ); io_wait();

   // The 8086 architecture uses IRQ line 2 to connect the master PIC to the slave PIC
   // IRQ line 2 is specified in master by 0b00000100
   outb(PIC_MASTER_DATA, (1 << 2)); io_wait(); // tell the master PIC slave's PIC at IR2.
   // IRQ line 2 is specified in slave by 0b00000010
   outb(PIC_SLAVE_DATA , (1 << 1)); io_wait(); // tell the slave its cascade identity

   // the pics should use the 8086 mode.
   outb(PIC_MASTER_DATA, ICW4_UPM); io_wait();
   outb(PIC_SLAVE_DATA , ICW4_UPM); io_wait();

   // restore the saved state or 'masks'
   outb(PIC_MASTER_DATA, master_data); io_wait();
   outb(PIC_SLAVE_DATA,  slave_data); io_wait();

   // Important! Limine masks these interrupts by default, we would have to unmask them.
   outb(PIC_MASTER_DATA, inb(PIC_MASTER_DATA) & ~(1 << 0));  // Unmask IRQ 0 (Timer) 
   outb(PIC_MASTER_DATA, inb(PIC_MASTER_DATA) & ~(1 << 1));  // Unmask IRQ 1 (Keyboard)
}

// 'masking' here means disabling by setting the bit to be 1.
void pic_disable_all_interrupts(void) {
   outb(PIC_MASTER_DATA, (s8)(u8)0xff); // THIS IS FINE
   outb(PIC_SLAVE_DATA, (s8)(u8)0xff); // THIS IS FINE
}

void pic_send_end_of_interrupt() {
   // set bit 5 of OCW 2
   outb(PIC_MASTER_COMMAND, (1<<5));
}
#endif
