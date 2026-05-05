; --- arch/i386/interrupts-a.asm ---

[BITS 32]

; --- Referencias externas en C ---
extern isr_handler      ; Tu manejador de excepciones en C
extern irq_handler      ; Tu manejador de hardware en C
extern keyboard_handler ; Tu driver de teclado universal

; --- Macros para Excepciones (ISR) ---
%macro ISR_NOERR 1
global isr%1
isr%1:
    cli                 ; Deshabilitar interrupciones
    push byte 0         ; Push un código de error ficticio (para mantener el stack simétrico)
    push byte %1        ; Push el número de la interrupción
    jmp isr_common_stub
%endmacro

%macro ISR_ERR 1
global isr%1
isr%1:
    cli
    ; El CPU ya pusheó el código de error por nosotros
    push byte %1        ; Push el número de la interrupción
    jmp isr_common_stub
%endmacro

; --- Macros para Hardware (IRQ) ---
%macro IRQ 2
global irq%1
irq%1:
    cli
    push byte 0         ; Código de error ficticio
    push byte %2        ; Número de interrupción (32-47)
    jmp irq_common_stub
%endmacro

; --- Generación de Excepciones (0-31) ---
ISR_NOERR 0  ; Divide by Zero
ISR_NOERR 1  ; Debug
ISR_NOERR 2  ; Non Maskable Interrupt
ISR_NOERR 3  ; Breakpoint
ISR_NOERR 4  ; Into Detected Overflow
ISR_NOERR 5  ; Out of Bounds
ISR_NOERR 6  ; Invalid Opcode
ISR_NOERR 7  ; No Coprocessor
ISR_ERR   8  ; Double Fault
ISR_NOERR 9  ; Coprocessor Segment Overrun
ISR_ERR   10 ; Bad TSS
ISR_ERR   11 ; Segment Not Present
ISR_ERR   12 ; Stack Fault
ISR_ERR   13 ; General Protection Fault (GPF)
ISR_ERR   14 ; Page Fault
ISR_NOERR 15 ; Unknown Interrupt
ISR_NOERR 16 ; x87 FPU Error
ISR_NOERR 17 ; Alignment Check
ISR_NOERR 18 ; Machine Check
ISR_NOERR 19 ; SIMD Floating Point
ISR_NOERR 20 ; Virtualization Exception
ISR_NOERR 21 ; Control Protection Exception
; ... del 22 al 31 son reservados ...
%assign i 22
%rep 10
    ISR_NOERR i
    %assign i i+1
%endrep

; --- Generación de IRQs (32-47) ---
IRQ 0, 32 ; Timer
IRQ 1, 33 ; Teclado (Llama a s390)
IRQ 2, 34 ; Cascada PIC
IRQ 3, 35 ; COM2
IRQ 4, 36 ; COM1
IRQ 5, 37 ; LPT2
IRQ 6, 38 ; Floppy
IRQ 7, 39 ; LPT1
IRQ 8, 40 ; Real Time Clock
IRQ 9, 41 ; ACPI
IRQ 10, 42 ; Libre / NIC
IRQ 11, 43 ; Libre / NIC (rtl8139)
IRQ 12, 44 ; PS2 Mouse
IRQ 13, 45 ; FPU
IRQ 14, 46 ; Primary ATA
IRQ 15, 47 ; Secondary ATA

; --- Common Stub para Excepciones ---
isr_common_stub:
    pushad              ; Guarda EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10        ; Carga el Kernel Data Segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call isr_handler    ; Llama a la lógica de pánico/error en C

    pop gs
    pop fs
    pop es
    pop ds
    popad
    add esp, 8          ; Limpia el código de error y el número de ISR
    sti
    iret

; --- Common Stub para IRQs (Hardware) ---
irq_common_stub:
    pushad
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; --- Lógica Especial para el Teclado ---
    mov eax, [esp + 48] ; Accedemos al número de interrupción en el stack
    cmp eax, 33         ; ¿Es el teclado (IRQ1)?
    jne .call_generic
    call keyboard_handler
    jmp .end_irq

.call_generic:
    call irq_handler    ; Llama a un manejador genérico si tienes uno

.end_irq:
    ; --- ENVIAR EOI AL PIC (Fundamental) ---
    mov al, 0x20
    out 0x20, al        ; Al Master PIC
    mov eax, [esp + 48]
    cmp eax, 40         ; Si es IRQ8-15, mandar EOI también al Slave
    jl .skip_slave
    out 0xA0, al
.skip_slave:

    pop gs
    pop fs
    pop es
    pop ds
    popad
    add esp, 8
    sti
    iret

; --- Wrapper para Syscalls ---
global syscall_isr_wrapper
extern syscall_handler
syscall_isr_wrapper:
    push byte 0
    push byte 0x80
    pushad
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
    iret
