[BITS 32]

section .multiboot
    align 4
    dd 0x1BADB002              ; Magic
    dd 0x00                    ; Flags
    dd - (0x1BADB002 + 0x00)   ; Checksum

section .text
global _start

_start:
    cli                         ; Apaga interrupciones
    lgdt [gdt_ptr]              ; Carga nuestra GDT

    ; EL SALTO MAESTRO:
    ; Esto carga 0x08 en CS y nos mueve a un entorno de 32 bits real.
    jmp 0x08:.full_32bit

.full_32bit:
    mov ax, 0x10                ; 0x10 es el offset del Data Segment en nuestra GDT
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Configura un stack limpio para evitar que pise el Initrd
    mov esp, stack_top

    push ebx                    ; Puntero a la estructura Multiboot que da Limine
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
    dw gdt_end - gdt_start - 1   ; Límite
    dd gdt_start                 ; Base

section .bss
align 16
stack_bottom:
    resb 16384                   ; 16 KB de stack
stack_top: