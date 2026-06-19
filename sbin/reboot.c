#include "libuser.h"

int main(int argc, char *argv[]) {
    user_print("GoodBye!");
    syscall(8, (int)argv[1]); 

    return 0;
}