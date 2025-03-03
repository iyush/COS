extern syscall_handler



global syscall_handler_wrapper
syscall_handler_wrapper:
  ;; this should be in opposite order of struct regs
	swapgs
	mov qword [gs:0016], rsp
	mov rsp, qword [gs:0008]

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
  call syscall_handler

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

	mov rsp, qword [gs:0016]

  o64 sysret
