; --- arch/i386/interrupt_entry.asm ---

[extern keyboard_handler] ; <--- Cambiado para que coincida con el .c

global keyboard_stub
keyboard_stub:
    pusha
    call keyboard_handler ; <--- Llamamos al nuevo nombre
    popa
    iret
