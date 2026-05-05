[BITS 32]
global gdt_flush
gdt_flush:
    mov eax, [esp + 4]
    lgdt [eax]
    mov ax, 0x10      ; Offset de Kernel Data
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:.flush   ; <--- ESTO PONE EL 0x08 EN CS
.flush:
    ret
