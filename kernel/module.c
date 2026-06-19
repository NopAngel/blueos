/*
 * BlueOS / kernel / module.c
 */

#include <kernel/colors.h>
#include <kernel/module.h>
#include <kernel/printk.h>
#include <lib/string.h>

#define MAX_MODULES 16
module_t *loaded_modules[MAX_MODULES];
int module_count = 0;

int sys_insmod(module_t *mod) {
  if (module_count >= MAX_MODULES)
    return -1;

  boot_msg("KMOD", "Loading module...\n", 0);
  if (mod->init) {
    if (mod->init() == 0) {
      loaded_modules[module_count++] = mod;
      boot_msg("KMOD", "Module loaded successfully\n", 0);
      return 0;
    }
  }

  boot_msg("KMOD", "Failed to initialize.\n", 2);
  return -1;
}

void sys_lsmod() {
  boot_msg("KMOD", "Loaded Modules:\n", 0);
  for (int i = 0; i < module_count; i++) {
    printk("KMOD   %s\n", 0, loaded_modules[i]->name);
  }
}