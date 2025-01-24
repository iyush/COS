.text

.global sysexit
.global _start

_start:
    xor %ebp, %ebp            # effectively RBP := 0, mark the end of stack frames
    mov (%rsp), %edi          # get argc from the stack (implicitly zero-extended to 64-bit)
    lea 8(%rsp), %rsi         # take the address of argv from the stack
    xor %eax, %eax            # per ABI and compatibility with icc
    call main                 # %edi, %rsi, %rdx are the three args (of which first two are C standard) to main

    mov %eax, %edi    # transfer the return of main to the first argument of _exit
    xor %eax, %eax    # per ABI and compatibility with icc
    call sysexit
