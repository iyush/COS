global start
global MULTIBOOT_TAG_PTR
extern c_start

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

section .text
bits 64


long_mode_start:
  ;; 64 bit mode already!!
  mov qword [MULTIBOOT_TAG_PTR], rbx
  call c_start

  hlt
