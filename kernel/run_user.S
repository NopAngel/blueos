global _run_user
_run_user:
    push ebp
    mov ebp, esp
    
    mov eax, [ebp + 8]      ; Entry point
    mov ebx, [ebp + 12]     ; argc
    mov ecx, [ebp + 16]     ; argv
    
    push ecx                ; Empujamos argv al stack para el main
    push ebx                ; Empujamos argc al stack para el main
    
    call eax                ; Llamada al binario (main)
    
    add esp, 8              ; Limpiamos
    pop ebp
    ret