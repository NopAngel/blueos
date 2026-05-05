; init.asm
[BITS 32]
start:
    mov ax, 0x10      ; Selector de datos
    mov ds, ax
    mov byte [0xb8000], '!' ; Escribe un '!' en la esquina de la pantalla
    mov byte [0xb8001], 0x0f
    hlt               ; Se detiene aquí
