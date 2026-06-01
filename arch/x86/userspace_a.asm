[BITS 32]

section .text
global asm_jump_to_user

asm_jump_to_user:
    mov ebp, esp
    mov ebx, [ebp+4]    ; Obtener la dirección de destino del argumento

    cli
    mov ax, 0x23        ; Segmento de datos de usuario
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push 0x23           ; SS de usuario
    push 0x004FFFF0     ; ESP de usuario
    pushf               ; EFLAGS
    pop eax
    or eax, 0x200       ; Habilitar interrupciones (IF)
    push eax
    push 0x1B           ; CS de usuario
    push ebx            ; EIP de usuario
    iret