// Reference: https://wiki.osdev.org/Inline_Assembly/Examples#I.2FO_access
#include "./io.h"

void outb(uint16_t port, uint8_t val) {
   asm volatile("outb %0, %1" :: "a"(val), "Nd"(port): "memory");
   // move 'val' to eax register.
   // Nd allows for one-byte constant values to be assembled as constants, freeing the edx registers
   // other cases. See: https://gcc.gnu.org/onlinedocs/gcc/Machine-Constraints.html
}

uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile ( "inb %1, %0"
                   : "=a"(ret)
                   : "Nd"(port)
                   : "memory");
    return ret;
}

// Wait a very small amount of time (1 to 4 microseconds, generally). Useful
// for implementing a small delay for PIC remapping on old hardware or
// generally as a simple but imprecise wait.
// You can do an IO operation on any unused port: the Linux kernel by default
// uses port 0x80, which is often used during POST to log information on the
// motherboard's hex display but almost always unused after boot. 
void io_wait(void)
{
    outb(0x80, 0);
}
