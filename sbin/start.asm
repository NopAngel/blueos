[bits 32]
section .text
global _start
extern main

_start:
    ; Llamamos a tu función main de C
    call main
    
    ; Si el main termina, salimos (Syscall 2)
    mov eax, 2
    int 0x80
    hlt