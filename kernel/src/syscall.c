#include "keyboard.h"

extern __attribute__((interrupt)) void syscall_handler_wrapper(RegsWithoutError *, u64);


void syscall_handler(RegsWithoutError * r) {
    switch(r->rax) {
        case 0: 
            {
                // cleanup();
                scheduler_cleanup_task();
            }
            break;
        case 1:
            {
                s32 size = (s32) r->rsi;
                ASSERT(size == r->rsi); // We make sure that we do not pass 64 bit number in the argument. output to console only supports 32 bit.
                framebuffer_log_write((char*)r->rdi, size);
                // output_to_console((char*)r->rdi, size);
            }
            break;
        case 2: // syscall: getchar
            {
                char c = keyboard_getchar();
                r->rax = (u64)c;
            }
            break;
        default:
            ksp("we received an generic rax %ld\n", r->rax);
            // dmpregs(*r);
    };
}


