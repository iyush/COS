#ifndef __ASA_BOCHS
#define __ASA_BOCHS
void bochs_breakpoint() {
    asm volatile ("xchgw %bx, %bx");
}
#endif
