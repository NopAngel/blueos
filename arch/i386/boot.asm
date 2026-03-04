[BITS 32]

section .multiboot
    align 4
    dd 0x1BADB002              ; Magic
    dd 0x00                    ; Flags
    dd - (0x1BADB002 + 0x00)   ; Checksum

section .text
global _start

_start:
    cli                         
    lgdt [gdt_ptr]              


    jmp 0x08:.full_32bit

.full_32bit:
    mov ax, 0x10                
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov esp, stack_top

    push ebx                   
    extern k_main
    call k_main

.halt:
    hlt
    jmp .halt

section .data
align 16
gdt_start:
    dq 0x0000000000000000        ; Null descriptor
    ; 0x08: Code Segment (Base=0, Limit=4GB, Type=Code/Read, DPL=0, 32-bit)
    dq 0x00cf9a000000ffff        
    ; 0x10: Data Segment (Base=0, Limit=4GB, Type=Data/Read/Write, DPL=0, 32-bit)
    dq 0x00cf92000000ffff        
gdt_end:

gdt_ptr:
    dw gdt_end - gdt_start - 1   
    dd gdt_start                 ; Base

section .bss
align 16
stack_bottom:
    resb 16384                   ; 16 KB stack
stack_top: