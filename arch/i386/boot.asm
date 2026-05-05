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

    ; Salto largo para establecer CS = 0x08
    jmp 0x08:.reload_segments

.reload_segments:
    ; Usamos ECX para cargar los segmentos y no tocar EAX ni EBX/EDX
    mov ecx, 0x10
    mov ds, cx
    mov es, cx
    mov fs, cx
    mov gs, cx
    mov ss, cx

    ; Configurar stack alineado
    mov esp, stack_top
    and esp, -16

    ; PASO DE PARÁMETROS LIMPIO:
    ; QEMU nos dio Magic en EAX y MBI en EBX.
    ; k_main con regparm(3) espera: EAX (arg1), EDX (arg2)

    mov edx, ebx        ; Movemos MBI a EDX sin pasar por el stack

    ; --- DEBUG EXTREMO (Opcional) ---
    ; Si quieres estar 100% seguro de que EAX no tiene basura:
    ; and eax, 0xFFFFFFFF
    ; --------------------------------

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
