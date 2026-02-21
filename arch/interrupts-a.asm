[EXTERN syscall_handler]
[GLOBAL syscall_isr_wrapper]

[GLOBAL idt_load]

idt_load:
    mov eax, [esp + 4]  ; Toma el puntero a idtp (el argumento)
    lidt [eax]          ; Carga la dirección real en el registro IDTR
    ret
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