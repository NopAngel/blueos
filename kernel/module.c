/*
 * BlueOS / kernel / module.c
 */

#include <kernel/module.h>
#include <kernel/printk.h>
#include <kernel/colors.h>
#include <lib/string.h>

#define MAX_MODULES 16
module_t *loaded_modules[MAX_MODULES];
int module_count = 0;


int sys_insmod(module_t *mod) {
    if (module_count >= MAX_MODULES) return -1;

    printk("\033[36m[KMOD] Loading module: %s...\033[0m\n", mod->name);
    
    if (mod->init) {
        if (mod->init() == 0) {
            loaded_modules[module_count++] = mod;
            printk("\033[32m\n[KMOD] Module %s loaded successfully.\033[0m\n", mod->name);
            return 0;
        }
    }
    
    printk("[KMOD] Failed to initialize %s.\n", mod->name);
    return -1;
}

void sys_lsmod() {
    printk("\nModule                  Size  Used by\n");
    for (int i = 0; i < module_count; i++) {
        printk("%-24s 4096  0\n", loaded_modules[i]->name);
    }
}