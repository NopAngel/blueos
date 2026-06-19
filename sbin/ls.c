#include "libuser.h"

int main(int argc, char **argv) {
    // Aquí necesitarías una syscall que reciba el nombre del dir
    // y el Kernel haga el printk directamente, porque los registros
    // no te dejan devolver estructuras complejas fácilmente.
    syscall(6, (int)(argc > 1 ? argv[1] : "/"));
    return 0;
}