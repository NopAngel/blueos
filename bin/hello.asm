[BITS 32]
[ORG 0x00400000]

start:
    ; sys_print(LIGHT_CYAN, msg)
    mov eax, 1          ; SYS_PRINTK
    mov ebx, msg        
    mov ecx, 11         
    int 0x80            

    ; sys_exit(0)
    mov eax, 3          ; SYS_EXIT
    mov ebx, 0          
    int 0x80

msg db "Hello World", 10, 0