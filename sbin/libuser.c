#include "stdint.h" // ¡Esto es lo que te falta!
#include "../include/kernel/syscall.h" 

void user_print(const char* str) {
    __asm__ volatile (
        "movl $1, %%eax\n"     // 1 es tu syscall de PRINT
        "movl %0, %%ebx\n"     // ebx = puntero al string
        "int $0x80\n"
        : 
        : "r"(str) 
        : "eax", "ebx"
    );
}
void user_exit() {
    __asm__ volatile ("int $0x80" : : "a"(2)); // 2 es SYS_EXIT
}

int syscall(int number, int arg1) {
    int ret;
    __asm__ volatile (
        "int $0x80"      // Interrupción al Kernel
        : "=a"(ret)      // Resultado en EAX
        : "a"(number), "b"(arg1) // Syscall ID en EAX, Argumento en EBX
    );
    return ret;
}

int vfs_chdir(const char *path) {
    int ret;
    __asm__ volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_CHDIR), "b"(path)
    );
    return ret;
}