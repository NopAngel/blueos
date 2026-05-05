#include <kernel/colors.h>
#include <kernel/printk.h>
#include <kernel/ports.h>
#include <kernel/panic.h>
#include <kernel/malloc.h>
#include <drivers/keyboard.h>
#include <drivers/ata.h>
#include <kernel/init_fnc.h>
#include <version.h>
#include <multiboot.h>
#include <lib/string.h>

/* --- Variables Globales --- */
int cursor_x = 0;
int cursor_y = 0;

/* --- Banner de BlueOS --- */
static void _blueos_banner() {
    clear_screen();
    printk(CYAN,  "  ____  _             \n");
    printk(CYAN,  " | __ )| |_   _  ___ "); printk(WHITE, "   Kernel: "); printk(GRAY, "%s\n", UTS_RELEASE);
    printk(CYAN,  " |  _ \\| | | | |/ _ \\"); printk(WHITE, "   Arch:   "); printk(GRAY, "%s\n", BLUEOS_ARCH);
    printk(CYAN,  " | |_) | | |_| |  __/\n");
    printk(CYAN,  " |____/|_|\\__,_|\\___|"); printk(WHITE, "   Built:  "); printk(GRAY, "%s\n", __DATE__);
    printk(CYAN, "\n --------------------------------------------------------------\n\n");
}

/**
 * k_main: Punto de entrada del kernel
 */
void k_main(unsigned int magic, multiboot_info_t* mbi) {
    asm volatile("cli");
    add_user("root", "123");

    gdt_init();
    idt_init();
    pic_init(0x20, 0x28);

    kmalloc_init(0x1000000, 1024 * 1024);
    tty_init(arch_put_char);
    _blueos_banner();

    asm volatile("sti");

    while (1) {
        asm volatile("hlt");
    }
}
