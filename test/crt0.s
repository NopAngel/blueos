.section .text
.global _start
.type _start, @function

_start:
    call main
    
    # Bucle infinito por si main retorna
1:  jmp 1b