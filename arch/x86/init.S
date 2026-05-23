; init.asm
[BITS 32]
start:
    mov ax, 0x10      
    mov ds, ax
    mov byte [0xb8000], '!' 
    mov byte [0xb8001], 0x0f
    hlt              
