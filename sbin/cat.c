#include "libuser.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        user_print("Use: cat <filename>\n");
        return -1;
    }
    syscall(8, (int)argv[1]); 

    return 0;
}