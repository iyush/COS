extern all_interrupts_handler

%macro INTERRUPT_WRAPPER 1
global int_wrapper_%1
int_wrapper_%1:
  ;; this should be in opposite order of struct regs
  push %1
  pushfq
  push r15
  push r14
  push r13
  push r12
  push r11
  push r10
  push r9
  push r8
  push rsp
  push rdi
  push rsi
  push rbp
  push rdx
  push rcx
  push rbx
  push rax
  ;; here we go.............
  cld
  mov rdi, rsp    ; Pass pointer to register structure as first argument
  call all_interrupts_handler

  pop rax
  pop rbx
  pop rcx
  pop rdx
  pop rbp
  pop rsi
  pop rdi
  pop rsp
  pop r8
  pop r9
  pop r10
  pop r11
  pop r12
  pop r13
  pop r14
  pop r15
  popfq

  ;; clean up the stack due to the interrupt number
  add rsp, 8

  iretq
%endmacro

INTERRUPT_WRAPPER 0
INTERRUPT_WRAPPER 1
INTERRUPT_WRAPPER 2
INTERRUPT_WRAPPER 3
INTERRUPT_WRAPPER 4
INTERRUPT_WRAPPER 5
INTERRUPT_WRAPPER 6
INTERRUPT_WRAPPER 7
INTERRUPT_WRAPPER 8
INTERRUPT_WRAPPER 9
INTERRUPT_WRAPPER 10
INTERRUPT_WRAPPER 11
INTERRUPT_WRAPPER 12
INTERRUPT_WRAPPER 13
INTERRUPT_WRAPPER 14
INTERRUPT_WRAPPER 16
INTERRUPT_WRAPPER 17
INTERRUPT_WRAPPER 18
INTERRUPT_WRAPPER 19
INTERRUPT_WRAPPER 20
INTERRUPT_WRAPPER 21

;; hardware interrupts
INTERRUPT_WRAPPER 32 ; timer
INTERRUPT_WRAPPER 33 ; keyboard


global int_wrapper_99
int_wrapper_99:
  ;; this should be in opposite order of struct regs
  push 99
  pushfq
  push r15
  push r14
  push r13
  push r12
  push r11
  push r10
  push r9
  push r8
  push rsp
  push rdi
  push rsi
  push rbp
  push rdx
  push rcx
  push rbx
  push rax
  ;; here we go.............
  cld
  mov rdi, rsp    ; Pass pointer to register structure as first argument
  call all_interrupts_handler

  pop rax
  pop rbx
  pop rcx
  pop rdx
  pop rbp
  pop rsi
  pop rdi
  pop rsp
  pop r8
  pop r9
  pop r10
  pop r11
  pop r12
  pop r13
  pop r14
  pop r15
  popfq

  ;; clean up the stack due to the interrupt number
  add rsp, 8

  o64 sysret



