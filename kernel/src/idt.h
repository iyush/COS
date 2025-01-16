#pragma once

#include "stdint.h"

struct __attribute__((packed)) regs {
   u64 rax;
   u64 rbx;
   u64 rcx;
   u64 rdx;
   u64 rbp;
   u64 rsi;
   u64 rdi;
   u64 rsp;
   u64 r8;
   u64 r9;
   u64 r10;
   u64 r11;
   u64 r12;
   u64 r13;
   u64 r14;
   u64 r15;

   u64 rflags;
   u64 interrupt_number;
   u64 error_code;
   u64 rip;
};


void init_idt(void);
