#ifndef VERSION_H
#define VERSION_H


#include "./printk.h"
#include "./colors.h"



extern int cursor_y;


const char* VERSION = "1.0";
const char *NAME_DEV = "BLUEKERNEL";
const char *NAME_COMPILED = "BlueOS Kernel";
const char *AUTHOR = "NopAngel";
const char *REPO = "https://github.com/NopAngel/bluekernel.git";
const char *LICENSE = "GPL-3.0";



int show_all_inf 
(void) 
{
    printk(NAME_DEV, cursor_y++, GREEN);
    printk(VERSION, cursor_y++, GREEN);
    printk(NAME_COMPILED, cursor_y++, GREEN);
    printk(AUTHOR, cursor_y++, GREEN);
    printk(REPO, cursor_y++, GREEN);
    printk(LICENSE, cursor_y++, GREEN);
}





#endif