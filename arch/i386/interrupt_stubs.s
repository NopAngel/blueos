
global irq0
global irq1
global irq2
global irq3
global irq4
global irq5
global irq6
global irq7
global irq8
global irq9
global irq10
global irq11
global irq12
global irq13
global irq14
global irq15
global exception_handler_stub
extern exception_handler

exception_handler_stub:
    pusha
    call exception_handler
    popa
    iret


global isr_common
isr_common:
    push byte 0      
    push byte 0      
    pusha           
    
    mov ax, ds
    push eax        

    mov ax, 0x10     
    mov ds, ax
    mov es, ax

    ;call exception_handler

    pop eax
    mov ds, ax
    mov es, ax
    popa
    add esp, 8
    iret

extern irq_handler


%macro IRQ 2
  irq%1:
    push byte 0      
    push byte %2     
    jmp irq_common_stub
%endmacro

IRQ 0, 32
IRQ 1, 33
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39
IRQ 8, 40
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

irq_common_stub:
    pusha                    
    mov ax, ds
    push eax               

    mov ax, 0x10             
    mov ds, ax
    mov es, ax

    push esp                 
    call irq_handler         
    add esp, 4               

    pop eax                
    mov ds, ax
    mov es, ax
    popa                     
    add esp, 8               
    iret