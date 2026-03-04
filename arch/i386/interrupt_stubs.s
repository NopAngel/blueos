; Indicamos que estas funciones son globales para que el Linker las vea
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

; Crea un punto de entrada común para las excepciones del 0 al 31
global isr_common
isr_common:
    push byte 0      ; Error code ficticio (algunas excepciones ya lo empujan, ojo ahí)
    push byte 0      ; Número de interrupción ficticio (puedes mejorar esto luego)
    pusha            ; Guarda registros
    
    mov ax, ds
    push eax         ; Guarda selector de datos

    mov ax, 0x10     ; Carga selector del Kernel
    mov ds, ax
    mov es, ax

    ;call exception_handler

    pop eax
    mov ds, ax
    mov es, ax
    popa
    add esp, 8
    iret

; Importamos la función de C que manejará la lógica
extern irq_handler

; Macro para no repetir código 16 veces
%macro IRQ 2
  irq%1:
    push byte 0      ; Error code ficticio
    push byte %2     ; Número de interrupción (IDT index)
    jmp irq_common_stub
%endmacro

; Definición de cada IRQ mapeada del 32 al 47
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
    pusha                    ; Guarda registros generales
    mov ax, ds
    push eax                 ; Guarda DS

    mov ax, 0x10             ; Carga el selector de datos del Kernel
    mov ds, ax
    mov es, ax

    push esp                 ; <--- ¡ESTO ES LA CLAVE! Empujas el puntero al struct
    call irq_handler         ; Llama a la función (ahora recibe un puntero)
    add esp, 4               ; Limpia el puntero empujado

    pop eax                  ; Restaura DS
    mov ds, ax
    mov es, ax
    popa                     ; Restaura registros generales
    add esp, 8               ; Limpia int_no y err_code
    iret