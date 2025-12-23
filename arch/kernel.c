// arch/kernel.c
#include "../include/colors.h"
#include "../include/printk.h"
#include "./ports.h"
#include "../include/panic.h"
#include "../include/drivers/keyboard.h"
#include "../include/fs/vfs.h"
#include "../include/fs/fs.h"

int cursor_x = 0;
int cursor_y = 0;

void splash_screen() 
{
    printk("*BlueOS* (kernel)", cursor_y++, WHITE);
    printk("type 'help' for a help", cursor_y++, WHITE);
}


int k_main(void)
{
    clear_screen();
    splash_screen();

    while (1) {
        keyboard_handler();       
    }
}
