; --- arch/x86/interrupt_entry.asm ---

[extern keyboard_handler] 

global keyboard_stub
keyboard_stub:
    pusha
    call keyboard_handler 
    popa
    iret
