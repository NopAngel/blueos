/*
 * BlueOS / kernel / sysctl.c
 */

#include <include/sysctl.h>
#include <include/lib/string.h>
#include <include/colors.h>
#include <include/printk.h>

int kernel_debug_level = 1;
char kernel_hostname[32] = "BlueOS-Machine";

sysctl_entry_t sysctl_table[] = {
    {"kernel.debug", &kernel_debug_level, 0, 1},
    {"kernel.hostname", kernel_hostname, 1, 1},
    {0, 0, 0, 0} /* Centinela */
};

void sysctl_list() {
    printk(WHITE, "\nAvailable kernel parameters:\n");
    for (int i = 0; sysctl_table[i].name != 0; i++) {
        printk(CYAN, "  %s = ", sysctl_table[i].name);
        if (sysctl_table[i].type == 0) 
            printk(WHITE, "%d\n", *(int*)sysctl_table[i].value);
        else 
            printk(WHITE, "%s\n", (char*)sysctl_table[i].value);
    }
}

int sysctl_set(const char *name, const char *new_value) {
    for (int i = 0; sysctl_table[i].name != 0; i++) {
        if (strcmp(sysctl_table[i].name, name) == 0) {
            if (!sysctl_table[i].writable) return -1;

            if (sysctl_table[i].type == 0) {
                /* Aquí podrías usar un atoi() si lo tienes */
                *(int*)sysctl_table[i].value = (new_value[0] - '0');
            } else {
                strncpy((char*)sysctl_table[i].value, new_value, 31);
            }
            return 0;
        }
    }
    return -2; /* No encontrado */
}