; switch.asm
global switch_to_task

switch_to_task:

    push ebp
    push edi
    push esi
    push ebx

    mov eax, [esp + 20]   
    mov [eax], esp

    mov esp, [esp + 24]     

    pop ebx
    pop esi
    pop edi
    pop ebp

    ret