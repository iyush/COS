#ifndef PIC_H
#define PIC_H
#include "./io.h"


#define PIC_MASTER_COMMAND   0x0020
#define PIC_MASTER_DATA      0x0021
#define PIC_SLAVE_COMMAND    0x00A0
#define PIC_SLAVE_DATA       0x00A1

#define ICW1_ICW4             (1<<0) // if this is set - icw4 has to be read. if icw4 is not needed
                                    // set ic4=0

#define ICW1_SNGL            (1<<1) // Means that this is the only 8259A in the system. If SNGL = 1,
                                    // no ICW3 will be issued.

#define ICW1_ADI             (1<<2) // Call address interval, if ADI = 1, then interval = 4, if ADI == 0,
                                    // then interval = 8.
                                  
#define ICW1_LTIM            (1<<3) // If LTIM = 1, then the 8259A will operate in
                                    // the level interrupt mode. Edge detect logic
                                    // on the interrupt inputs will be disabled.

#define ICW1_INIT            (1<<4) // Initialization

#define ICW4_8086	      0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	      0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	      0x10		/* Special fully nested (not) */
                                  
void pic_remap(uint8_t master_offset, uint8_t slave_offset) {
   // save the current state or "masks"
   uint8_t master_data = inb(PIC_MASTER_DATA);
   uint8_t slave_data = inb(PIC_SLAVE_DATA);

   // start the initialization with casecade mode.
   outb(PIC_MASTER_COMMAND, ICW1_INIT | ICW1_ICW4); io_wait();
   outb(PIC_SLAVE_COMMAND , ICW1_INIT | ICW1_ICW4); io_wait();

   // master and slave pic vector offset
   outb(PIC_MASTER_DATA, master_offset); io_wait();
   outb(PIC_SLAVE_DATA , slave_offset ); io_wait();

   outb(PIC_MASTER_DATA, 4); io_wait(); // tell the master PIC slave's PIC at IR2.
   outb(PIC_SLAVE_DATA , 2); io_wait(); // tell the slave its cascade identity

   // the pics should use the 8086 mode.
   outb(PIC_MASTER_DATA, ICW4_8086); io_wait();
   outb(PIC_SLAVE_DATA , ICW4_8086); io_wait();

   // restore the saved state or 'masks'
   outb(PIC_MASTER_DATA, master_data); io_wait();
   outb(PIC_SLAVE_DATA,  slave_data); io_wait();
}
#endif
