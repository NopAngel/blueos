#include "libuser.h"

int main(int argc, char **argv) {
    if (argc < 2) return 1;
    // Syscall 3 = vfs_mkdir
    syscall(5, (int)argv[1]);
    return 0;
}