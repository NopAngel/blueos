[BITS 32]
[ORG 0x00400000] ; La dirección donde el loader lo pone

start:
    ; sys_print(LIGHT_CYAN, msg)
    mov eax, 1          ; SYS_PRINTK (según tu syscall.c)
    mov ebx, msg        ; Puntero al mensaje
    mov ecx, 11         ; Color (LIGHT_CYAN)
    int 0x80            ; ¡SYSCALL!

    ; sys_exit(0)
    mov eax, 3          ; SYS_EXIT
    mov ebx, 0          ; Código de salida 0
    int 0x80

msg db "Hello BlueOS! This is pure Assembly userspace, bro!", 10, 0