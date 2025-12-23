#include "../include/printk.h"
#include "../include/drivers/keyboard.h"
#include "../include/colors.h"
#include "../include/bg.h"

extern int cursor_y;

int k_panic (char* msg)
{
    bg_clear(WHITE_RED);    
    printk("KERNEL Panic", cursor_y++, WHITE_RED);
    printk(msg, cursor_y++, WHITE_RED);
    while (1) 
    {
        __asm__ volatile ("hlt");
    }
}
