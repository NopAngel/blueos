#include <blueos/printk.h>
#include <blueos/panic.h>

extern char* stats;
extern int cursor_y;

void snd_init_core ()
{
    printk(stats, cursor_y++, WHITE);
    return 0;
}

void snd_crash()
{
   k_panic("Sound Core - CRASH");
}
