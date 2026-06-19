#include "libuser.h"

int main(int argc, char *argv[]) {
    if (argc > 1) {
        user_print(argv[1]);
    } else {
        user_print("err: you didn't present any arguments.");
    }

    return 0;
}