; --- BlueOS Kernel Loader ---
[BITS 32]

; --- Multiboot Header ---
SECTION .multiboot
    align 4
    MULTIBOOT_MAGIC    equ 0x1BADB002
    MULTIBOOT_FLAGS    equ 0x00000003 ; ALIGN + MEMINFO
    CHECKSUM           equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd CHECKSUM

SECTION .text
global _start
extern k_main

_start:
    cli
    lgdt [gdt_ptr]

    jmp 0x08:.reload_segments

.reload_segments:
    mov ecx, 0x10
    mov ds, cx
    mov es, cx
    mov fs, cx
    mov gs, cx
    mov ss, cx

    mov esp, stack_top
    and esp, -16


    mov edx, ebx

    push ebx
    push eax
    extern k_main
    call k_main

.halt:
    cli
    hlt
    jmp .halt

; --- GDT ---
SECTION .data
align 16
gdt_start:
    dq 0x0000000000000000        ; Null
    dq 0x00cf9a000000ffff        ; Code 0x08
    dq 0x00cf92000000ffff        ; Data 0x10
gdt_end:

gdt_ptr:
    dw gdt_end - gdt_start - 1
    dd gdt_start

SECTION .bss
align 16
stack_bottom:
    resb 16384                   ; 16 KB
stack_top:
