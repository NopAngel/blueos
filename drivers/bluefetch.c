#include <kernel/module.h>
#include <blueos/printk.h>
#include <version.h>
#include <stddef.h>
#include <stdint.h>

#ifndef LIGHT_RED
    #define LIGHT_RED      12
    #define YELLOW         14
    #define GREEN          2
    #define CYAN           3
    #define LIGHT_BLUE     9
    #define LIGHT_MAGENTA  13
    #define WHITE          15
#endif

extern char current_user[32];
extern uint64_t total_memory_kb;
extern uint64_t used_memory_kb;
void print_raccoon_real() {
    int r[] = {LIGHT_RED, YELLOW, GREEN, CYAN, LIGHT_BLUE, LIGHT_MAGENTA, WHITE};


    printk(r[0], "                   __        .-. \n");
    printk(r[1], "               .-\"` .`'.    /\\\\\\| \n");
    printk(r[2], "       _(\\-/)\" ,  .   ,\\  /\\\\\\\\/ \n");
    printk(r[3], "      {(#b^d#)} .   ./,  |/\\\\\\\\/   "); printk(WHITE, " User:   "); printk(CYAN, "%s@users\n", current_user);
    printk(r[4], "      `-.(Y).-`  ,  |  , |\\.-`     "); printk(WHITE, " OS:     "); printk(CYAN, "%s %s\n", BLUEOS_NAME, UTS_RELEASE);
    printk(r[5], "           /~/,_/~~~\\,__.-`        "); printk(WHITE, " Kernel: "); printk(CYAN, "%s\n", UTS_RELEASE);
    printk(r[6], "          ////~    // ~\\\\          "); printk(WHITE, " Arch:   "); printk(CYAN, "%s\n", BLUEOS_ARCH);
    printk(r[0], "        ==`==`   ==`   ==`         "); printk(WHITE, " RAM:    "); printk(CYAN, "%dMB / %dMB\n", used_memory_kb/1024, total_memory_kb/1024);
    display_system_palette();
}


int bluefetch_init(void) { return 0; }
void bluefetch_exit(void) { }

MODULE_INFO("bluefetch", bluefetch_init, bluefetch_exit);