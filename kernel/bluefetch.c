#include <drivers/soc_intel.h>
#include <kernel/module.h>
#include <kernel/printk.h>
#include <stddef.h>
#include <stdint.h>
#include <version.h>

extern soc_info_t sys_soc;

#ifndef LIGHT_RED
#define LIGHT_RED 12
#define YELLOW 14
#define GREEN 2
#define CYAN 3
#define LIGHT_BLUE 9
#define LIGHT_MAGENTA 13
#define WHITE 15
#endif

extern char current_user[32];
extern unsigned int total_memory_kb;
extern uint64_t used_memory_kb;
void __logo_art_ascii() {
  extern int tty_current();
  printk("\033[91m                   __        .-. \n");
  printk("\033[33m               .-\"` .`'.    /\\\\\\| \n");
  printk("\033[32m       _(\\-/)\" ,  .   ,\\  /\\\\\\\\/ \033[36m "
         "-----------------------\n");
  printk("\033[36m      {(#b^d#)} .   ./,  |/\\\\\\\\/    %s@users\n",
         current_user);
  printk("\033[34m      `-.(Y).-`  ,  |  , |\\.-`      \033[37m OS:     "
         "\033[36m %s %s\n",
         BLUEOS_NAME, UTS_RELEASE);
  printk("\033[35m           /~/,_/~~~\\,__.-`         \033[37m Kernel: "
         "\033[36m %s\n",
         UTS_RELEASE);
  printk("\033[37m          ////~    // ~\\\\           \033[37m Arch:   "
         "\033[36m %s\n",
         BLUEOS_ARCH);
  printk("\033[91m        ==`==`   ==`   ==`          \033[37m TTY:    "
         "\033[36m %d\n\033[0m",
         tty_current());
}

int bluefetch_init(void) { return 0; }
void bluefetch_exit(void) {}

MODULE_INFO("bluefetch", bluefetch_init, bluefetch_exit);
