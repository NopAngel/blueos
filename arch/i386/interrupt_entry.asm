[extern keyboard_handler]

global keyboard_stub
keyboard_stub:
    pusha           ; Guarda los registros del programa que interrumpimos
    call keyboard_handler ; Llama a tu función de C
    popa            ; Restaura los registros
    iret            ; Instrucción especial para volver de interrupción