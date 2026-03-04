[EXTERN syscall_handler]
[GLOBAL syscall_isr_wrapper]
[EXTERN keyboard_handler]
global keyboard_wrapper
keyboard_wrapper:
    pusha               ; save EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
    
    call keyboard_handler 
    
    popa                
    iret              

syscall_isr_wrapper:
    cli             
    push 0         
    push 0x80      
    
    pushad          ; save EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
    push ds
    push es
    push fs
    push gs
    
    mov ax, 0x10    
    mov ds, ax
    mov es, ax
    
    call syscall_handler
    
    pop gs
    pop fs
    pop es
    pop ds
    popad
    add esp, 8      
    sti            
    iretd           


global keyboard_asm_handler  
extern keyboard_handler

keyboard_asm_handler:
    pushad
    call keyboard_handler
    popad
    iretd