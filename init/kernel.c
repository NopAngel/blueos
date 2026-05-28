#include <kernel/colors.h>
#include <kernel/printk.h>
#include <kernel/panic.h>
#include <kernel/malloc.h>
#include <drivers/keyboard.h>
#include <drivers/tty.h>
#include <drivers/vt220.h>
#include <mm/memory.h>
#include <fs/vfs.h>
#include <version.h>
#include <lib/string.h>
#include <auth.h>
#include <kernel/arch.h> 
extern int tty_current();

int cursor_x = 0;
int cursor_y = 0;

extern void init_all(void* arch_data);

void _blueos_banner() {
    printk(CYAN,  "  ____  _             \n");
    printk(CYAN,  " | __ )| |_   _  ___ "); printk(WHITE, "   Kernel: "); printk(GRAY, "%s\n", UTS_RELEASE);
    printk(CYAN,  " |  _ \\| | | | |/ _ \\"); printk(WHITE, "   Arch:   "); printk(GRAY, "%s\n", BLUEOS_ARCH);
    printk(CYAN,  " | |_) | | |_| |  __/\\"); printk(WHITE, "  TTY:    "); printk(GRAY, "tty%d\n", tty_current());
    printk(CYAN,  " |____/|_|\\__,_|\\___|"); printk(WHITE, "   Built:  "); printk(GRAY, "%s\n", __DATE__);
    printk(CYAN, "\n --------------------------------------------------------------\n\n");
}

void k_main(unsigned int magic, void* arch_data) {
    init_all(arch_data);
    _blueos_banner();

    while (1) {
        arch_idle();
    }

    
}
