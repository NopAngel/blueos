[BITS 32]
section .text
global _syscall

; generic function: _syscall(num, arg1, arg2, arg3)
_syscall:
    push ebp
    mov ebp, esp
    push ebx

    mov eax, [ebp + 8]  ; num
    mov ebx, [ebp + 12] ; arg1
    mov ecx, [ebp + 16] ; arg2
    mov edx, [ebp + 20] ; arg3
    int 0x80            ; 

    pop ebx
    mov esp, ebp
    pop ebp
    ret