global start
global MULTIBOOT_TAG_PTR
extern c_start
extern pic_send_end_of_interrupt

section .text
bits 32
start:
  ;; We are doing identity mapping here
  ;; p4[0] -> p3[0]
  mov eax, p3_table
  or eax, 011b                  ; setting flag: present and writable
  mov dword [p4_table+0], eax

  ;; p3[0] -> p2[0]
  mov eax, p2_table
  or eax, 011b                  ; setting flag: present and writable
  mov dword [p3_table+0], eax

  ;; mapping p2 to actual pages
  mov ecx, 0                    ; counter var
.map_p2_table:
  mov eax, 0x200000             ; This is 2MiB
  mul ecx
  or eax, 010000011b            ; setting flag: present, writable and granularity of 2MiB
                                ; would be 4KiB if not set.
  mov [p2_table + ecx * 8], eax
  inc ecx
  cmp ecx, 512                  ; Page table is 4096 bytes with each entry 8
                                ; bytes which means 512 entries. 512 entries *
                                ; 2 MiB => 1 GiB of pageable memory
  jne .map_p2_table


  ;; Now we are enabling pages
  ;; --------------------------

  ;; 1. Put address of the p4 table to special register
  mov eax, p4_table
  mov cr3, eax

  ;; 2. Enable PAE
  mov eax, cr4
  or eax, 1 << 5
  mov cr4, eax

  ;; 3. Setting long mode bit
  mov ecx, 0xC0000080
  rdmsr
  or eax, 1 << 8
  wrmsr

  ;; 4. Enable Paging
  mov eax, cr0
  or eax, 1 << 31
  or eax, 1 << 16
  mov cr0, eax
  ;;- ---------------------

  ;; load gdt
  lgdt [gdt64.pointer]

  ;; update selectors
  mov ax, gdt64.data         ; kinda casting to 16 bit as segment registers are 16 bit
  mov ss, ax                 ; stack segment register
  mov ds, ax                 ; data segment register
  mov es, ax                 ; extra segment register

  ;; now we update code segment register
  ;; we cannot modify code segment directly by 'mov cx, ax'
  ;; We need to execute a 'far jump' to modify it.
  ;; Now we enter 64 bit mode!
  jmp gdt64.code:long_mode_start

section .bss
align 4096

p4_table:
  resb 4096
p3_table:
  resb 4096
p2_table:
  resb 4096

section .bss
MULTIBOOT_TAG_PTR:
  resb 64

section .rodata
gdt64:
    dq 0
.code: equ $ - gdt64
    dq (1<<44) | (1<<47) | (1<<41) | (1<<43) | (1<<53)
.data: equ $ - gdt64
    dq (1<<44) | (1<<47) | (1<<41)
.pointer:
    dw .pointer - gdt64 - 1
    dq gdt64


section .bss
regs:
  resb 2048

section .text
bits 64

%macro INTERRUPT_WRAPPER 2
global int_wrapper_%1
extern %2
int_wrapper_%1:
  ;; this should be in opposite order of struct regs
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
  ; push rip
  ; push cs
  ; push ds
  ; push ss
  ; push fs
  ; push gs

  ;; here we go.............
  cld
  call %2

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
  ; pop rip

  ; pop cs
  ; pop ds
  ; pop ss
  ; pop fs
  ; pop gs
  mov rax, %1
  ;; call pic_send_end_of_interrupt
  iretq
%endmacro

long_mode_start:
  ;; 64 bit mode already!!
  mov qword [MULTIBOOT_TAG_PTR], rbx
  call c_start

  hlt

INTERRUPT_WRAPPER 0,  divide_error                 
INTERRUPT_WRAPPER 1,  debug                        
INTERRUPT_WRAPPER 2,  nmi_interrupt                
INTERRUPT_WRAPPER 3,  breakpoint                   
INTERRUPT_WRAPPER 4,  overflow                     
INTERRUPT_WRAPPER 5,  bound_range_exceeded         
INTERRUPT_WRAPPER 6,  invalid_opcode               
INTERRUPT_WRAPPER 7,  device_not_available         
INTERRUPT_WRAPPER 8,  double_fault                 
INTERRUPT_WRAPPER 9,  co_processor_segment_overrun 
INTERRUPT_WRAPPER 10, invalid_tss                  
INTERRUPT_WRAPPER 11, segment_not_present          
INTERRUPT_WRAPPER 12, stack_segment_fault          
INTERRUPT_WRAPPER 13, general_protection           
INTERRUPT_WRAPPER 14, page_fault                   
INTERRUPT_WRAPPER 16, floating_point_error         
INTERRUPT_WRAPPER 17, alignment_check              
INTERRUPT_WRAPPER 18, machine_check                
INTERRUPT_WRAPPER 19, simd_floating_point_exception
INTERRUPT_WRAPPER 20, virtualization_exception 
INTERRUPT_WRAPPER 21, control_protection_exception

;; hardware interrupts
INTERRUPT_WRAPPER 32, timer
