[EXTERN syscall_handler]
[GLOBAL syscall_isr_wrapper]
[EXTERN keyboard_handler]
global keyboard_wrapper
keyboard_wrapper:
    pusha               ; Guarda EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
    
    call keyboard_handler ; Llama a tu código de C
    
    popa                ; Restaura los registros
    iret                ; ¡FUNDAMENTAL! Vuelve de la interrupción correctamente

syscall_isr_wrapper:
    cli             ; Desactiva interrupciones
    push 0          ; Código de error ficticio
    push 0x80       ; Número de interrupción
    
    pushad          ; Guarda EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
    push ds
    push es
    push fs
    push gs
    
    mov ax, 0x10    ; Carga el selector de datos del kernel
    mov ds, ax
    mov es, ax
    
    call syscall_handler
    
    pop gs
    pop fs
    pop es
    pop ds
    popad
    add esp, 8      ; Limpia el código de error y el num de interrupción
    sti             ; Reactiva interrupciones
    iretd           ; Vuelve al modo usuario


global keyboard_asm_handler  ; <--- ESTO ES VITAL
extern keyboard_handler

keyboard_asm_handler:
    pushad
    call keyboard_handler
    popad
    iretd