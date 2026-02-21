/*
 * BlueOS / kernel / module.c
 */

#include <include/kernel/module.h>
#include <include/printk.h>
#include <include/colors.h>
#include <include/lib/string.h>

#define MAX_MODULES 16
module_t *loaded_modules[MAX_MODULES];
int module_count = 0;


int sys_insmod(module_t *mod) {
    if (module_count >= MAX_MODULES) return -1;

    printk(CYAN, "[KMOD] Loading module: %s...\n", mod->name);
    
    if (mod->init) {
        if (mod->init() == 0) {
            loaded_modules[module_count++] = mod;
            printk(GREEN, "\n[KMOD] Module %s loaded successfully.\n", mod->name);
            return 0;
        }
    }
    
    printk(RED, "[KMOD] Failed to initialize %s.\n", mod->name);
    return -1;
}

void sys_lsmod() {
    printk(WHITE, "\nModule                  Size  Used by\n");
    for (int i = 0; i < module_count; i++) {
        printk(WHITE, "%-24s 4096  0\n", loaded_modules[i]->name);
    }
}