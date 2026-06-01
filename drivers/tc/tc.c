#include <drivers/tc.h>
#include <kernel/printk.h>
#include <kernel/colors.h>

static struct tc_settings current_tc;

void tc_init() {
    current_tc.flags = TC_ECHO | TC_ICANON | TC_ISIG;
    current_tc.erase_char = '\b';
    current_tc.kill_char = 0x15; // Ctrl+U
    printk("[TC] Terminal Control Driver active.\n");
}

void tc_handle_input(char c) {
    if ((current_tc.flags & TC_ISIG) && c == 0x03) {
        printk("\n^C - Interrupt sent to process.\n");
        return;
    }

    if (c == '\b' || c == 0x7F) {
        if (current_tc.flags & TC_ECHO) {
            printk("\b \b");
        }
        return;
    }

    // 3. Eco (Echo)
    if (current_tc.flags & TC_ECHO) {
        char str[2] = {c, '\0'};
        printk("%s", str);
    }
}


void tc_set_color(const char *ansi_color) {
    printk("%s", ansi_color);
}
