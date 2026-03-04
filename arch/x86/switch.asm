[BITS 64]
global switch_to_task

section .text

switch_to_task:
    ; Argumentos recibidos por registros (System V ABI):
    ; RDI = uint64_t* old_rsp_ptr (Donde guardaremos el stack actual)
    ; RSI = uint64_t  new_rsp     (El valor del nuevo stack)

    ; 1. Guardar registros "callee-saved" (los que el Kernel debe preservar)
    ; En x64 son: RBX, RBP, R12, R13, R14, R15
    push rbp
    push rbx
    push r12
    push r13
    push r14
    push r15

    ; 2. Guardar el RSP actual en la dirección que apunta RDI
    ; Es decir: *old_rsp_ptr = rsp;
    mov [rdi], rsp

    ; 3. Cargar el nuevo RSP desde RSI
    ; Es decir: rsp = new_rsp;
    mov rsp, rsi

    ; 4. Restaurar registros de la NUEVA tarea (en orden inverso al push)
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp

    ; 5. Volver a la dirección de retorno (RIP) que está en el tope del nuevo stack
    ret