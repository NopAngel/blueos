; switch.asm
global switch_to_task

switch_to_task:
    ; Los argumentos en el stack están en:
    ; [esp + 4] -> uint32_t** old_esp (donde guardaremos el stack actual)
    ; [esp + 8] -> uint32_t  new_esp (el stack al que queremos saltar)

    ; 1. Guardar registros del contexto actual
    push ebp
    push edi
    push esi
    push ebx

    ; 2. Guardar el ESP actual en la dirección apuntada por el primer argumento
    mov eax, [esp + 20]     ; 20 = 4 (ret) + 16 (los 4 push de arriba)
    mov [eax], esp

    ; 3. Cambiar al nuevo ESP (segundo argumento)
    mov esp, [esp + 24]     ; 24 = 8 (original) + 16 (de los push)

    ; 4. Restaurar registros de la NUEVA tarea
    pop ebx
    pop esi
    pop edi
    pop ebp

    ret