#include <include/printk.h>
#include <include/drivers/keyboard.h>
#include <include/colors.h>
#include <include/bg.h>

extern int cursor_y;

int k_panic (char* msg)
{
    bg_clear(WHITE_RED);
    printk("== Kernel Panic ==", cursor_y++, WHITE_RED);
    printk("System error - Crash kernel", cursor_y++, WHITE_RED);
    cursor_y++;
    printk(msg, cursor_y++, WHITE_RED);


    while(1) {
            __asm__ volatile (
                "cli\n"
                "hlt\n"
                "jmp .-2"
            );
    }
}
