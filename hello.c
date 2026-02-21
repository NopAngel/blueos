/* BlueOS / hello.c */
#include <include/kernel/module.h>
#include <include/printk.h>
#include <include/colors.h>

int hello_init(void) {
    printk(CYAN, "[MODULE] ");
    printk(WHITE, "Hello, NopAngel! Soporte LKM activo.\n");
    return 0;
}

void hello_exit(void) {
    printk(WHITE, "Modulo hello descargado.\n");
}


module_t __this_module = {
    .name = "hello_world",
    .init = hello_init,
    .exit = hello_exit,
    .description = "Primer modulo de BlueOS",
    .version = "1.0"
};