/*
 * BlueOS arch/riscv/kernel.c
 * Core Kernel Entry Point for RISC-V.
 */

#include <kernel/colors.h>
#include <kernel/printk.h>
#include <kernel/panic.h>
#include <kernel/task.h>
#include <drivers/keyboard.h>
#include <kernel/init_fnc.h>
#include <version.h>
#include <fs/vfs.h>

extern struct task_struct *tasks[];
int cursor_x;
int cursor_y;

#define KERNEL_STACK_SIZE 4096
uint8_t kernel_stack_buffer[KERNEL_STACK_SIZE] __attribute__ ((aligned(16)));

uintptr_t get_kernel_stack_top() {
    return (uintptr_t)kernel_stack_buffer + KERNEL_STACK_SIZE;
}

enum system_states {
    SYSTEM_BOOTING,
    SYSTEM_RUNNING,
    SYSTEM_PANIC
} system_state;

static uint64_t read_cycles() {
    uint64_t cycles;
    __asm__ __volatile__ ("rdtime %0" : "=r"(cycles));
    return cycles;
}

static void _blueos_banner() {
    printk(WHITE, "\033[2J\033[H");

    printk(CYAN,  "  ____  _             \n");
    printk(CYAN,  " | __ )| |_   _  ___ "); printk(WHITE, "   Kernel: "); printk(GRAY, "%s\n", UTS_RELEASE);
    printk(CYAN,  " |  _ \\| | | | |/ _ \\"); printk(WHITE, "   Arch:   "); printk(GRAY, "%s\n", BLUEOS_ARCH);
    printk(CYAN,  " | |_) | | |_| |  __/\n");
    printk(CYAN,  " |____/|_|\\__,_|\\___|"); printk(WHITE, "   UTS VERSION:    "); printk(GRAY, "%s\n", UTS_VERSION);
    
    printk(CYAN, "\n --------------------------------------------------------------\n\n");
}

static void rest_init() {

    uint64_t t = read_cycles();
    
    printk(WHITE, "\n[ System Initialized ]\n");
    printk(GRAY, " Clock Ticks since boot: %d\n", t);
    printk(WHITE, "BlueOS %s-riscv ttyUART0\n\n", UTS_RELEASE);

    print_prompt();
}



void k_main(uintptr_t hartid, uintptr_t dtb_ptr) {
    uintptr_t stack_top = get_kernel_stack_top();
    __asm__ __volatile__ ("mv sp, %0" : : "r"(stack_top));

    system_state = SYSTEM_BOOTING;

    init_all();
    _blueos_banner();
    
    system_state = SYSTEM_RUNNING;
    rest_init();

    while (1) {
        #ifdef CONFIG_DRIVER_KEYBOARD
            keyboard_handler(); 
        #endif
    }
}