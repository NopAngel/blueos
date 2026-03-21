#include <kernel/initcall.h>
#include <kernel/printk.h>
#include <kernel/colors.h>

extern initcall_t __initcall_start;
extern initcall_t __initcall_end;

void do_initcalls(void) {
    initcall_t *call = &__initcall_start;
    
    printk(CYAN, "Kernel: Execution of initcalls starting...\n");

    for (; call < &__initcall_end; call++) {
        if (*call) {
            int result = (*call)();
            if (result != 0) {
                printk(RED, "Initcall failed with error code: %d\n", result);
            }
        }
    }
}