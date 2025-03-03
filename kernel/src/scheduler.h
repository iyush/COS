#ifndef __ASA_SCHEDULER
#define  __ASA_SCHEDULER

typedef struct RegsWithError {
   s64 rax;
   s64 rbx;
   s64 rcx;
   s64 rdx;
   s64 rbp;
   s64 rsi;
   s64 rdi;
   s64 r8;
   s64 r9;
   s64 r10;
   s64 r11;
   s64 r12;
   s64 r13;
   s64 r14;
   s64 r15;

   s64 error_code;
   s64 rip;
   s64 cs;
   s64 rflags;
   s64 rsp;
   s64 ss;
} __attribute__((packed)) RegsWithError;

typedef struct RegsWithoutError {
   s64 rax;
   s64 rbx;
   s64 rcx;
   s64 rdx;
   s64 rbp;
   s64 rsi;
   s64 rdi;
   s64 r8;
   s64 r9;
   s64 r10;
   s64 r11;
   s64 r12;
   s64 r13;
   s64 r14;
   s64 r15;

   s64 rip;
   s64 cs;
   s64 rflags;
   s64 rsp;
   s64 ss;
} __attribute__((packed)) RegsWithoutError;

void scheduler_cleanup_task();
void scheduler_preempt_and_schedule(RegsWithoutError* r);
#endif
