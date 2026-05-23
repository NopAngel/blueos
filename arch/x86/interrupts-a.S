; --- arch/x86/interrupts-a.asm ---

[BITS 32]

extern isr_handler      
extern irq_handler      
extern keyboard_handler 
extern mouse_handler

%macro ISR_NOERR 1
global isr%1
isr%1:
    cli                 
    push byte 0         
    push byte %1        
    jmp isr_common_stub
%endmacro

%macro ISR_ERR 1
global isr%1
isr%1:
    cli
    push byte %1        
    jmp isr_common_stub
%endmacro

%macro IRQ 2
global irq%1
irq%1:
    cli
    push byte 0         
    push byte %2        
    jmp irq_common_stub
%endmacro

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

%assign i 22
%rep 10
    ISR_NOERR i
    %assign i i+1
%endrep

IRQ 0, 32 ; Timer
IRQ 1, 33 ; keyboard (s390)
IRQ 2, 34 ; PIC
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

isr_common_stub:
    pushad              ; save EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10        
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call isr_handler    

    pop gs
    pop fs
    pop es
    pop ds
    popad
    add esp, 8          
    sti
    iret

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

    
    mov eax, [esp + 48]

    cmp eax, 33
    jne .call_generic
    call keyboard_handler

    cmp eax, 44
    jne .call_generic
    call mouse_handler
    jmp .end_irq

.call_generic:
    push esp           
    call irq_handler
    add esp, 4          

.end_irq:
    mov al, 0x20
    out 0x20, al        ; Al Master PIC
    mov eax, [esp + 48]
    cmp eax, 40         
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
