#include "libuser.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        vfs_chdir("/");
    } else {
        if (vfs_chdir(argv[1]) != 0) {
            user_print("error: no such directory\n");
        }
    }
    
    // ¡CRUCIAL! Salida segura
    syscall(2, 0); 
    return 0;
}