#include <lib/string.h>
#include <stdint.h>
#include <kernel/printk.h>
#include <kernel/colors.h>

/* External references from commands.c */
extern void execute_shell_command(char* input);
extern void print_prompt();

/* This function can be called from kmain or after login */
void init_shell() {
    clear_screen();
    printk("BlueOS Shell Interface\n");
    printk("Type 'help' to see available commands.\n\n");
    print_prompt();
}

/* The logic for actually 'running' the shell is handled by
   interrupts in keyboard.c, which calls execute_shell_command().

   You can keep display_system_palette here if you use it
   for debugging colors.
*/
void display_system_palette() {
    for (int i = 0; i < 8; i++) {
        vt100_set_color(i);
        printk("%c%c", 219, 219);
    }
    vt100_set_color(0x0F);
    printk("\n  ");
    for (int i = 8; i < 16; i++) {
        vt100_set_color(i);
        printk("%c%c", 219, 219);
    }
    vt100_set_color(0x0F);
    printk("\n\n");
}
