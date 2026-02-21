/*
 * BlueOS arch/kernel.c
 *
 * Copyright (C) 2024-2026  NopAngel <angelgabrielnieto@outlook.com>
 *
 * This is the main entry point for the BlueOS Kernel.
 * Based on the architecture of the Linux start_kernel sequence.
 * * "Any OS that doesn't boot into a shell is just a bootloader."
 */

#include <include/colors.h>
#include <include/printk.h>
#include <include/ports.h>
#include <include/panic.h>
#include <include/init_fnc.h>
#include <include/drivers/keyboard.h>
#include <include/fs/vfs.h>
#include <include/fs/fs.h>
#include <include/version.h>
#include <include/interrupts.h>
#include <include/multilru.h>
#include <include/profile.h>
#include <include/auth.h>
#include <include/task.h>
#include <include/sysfs.h>
#include <include/sysctl.h>
#include <include/kernel/module.h>
/* System states, similar to those found in linux/include/linux/kernel.h */
enum system_states {
    SYSTEM_BOOTING,
    SYSTEM_RUNNING,
    SYSTEM_PANIC
} system_state;

page_t system_page;
page_t user_page;
extern int current_user_index; /* Global user session state */
extern module_t __this_module;
/**
 * _blueos_banner - Print kernel version and build information.
 * * Equivalent to the early pr_notice() calls in Linux's start_kernel().
 */

extern char current_user[32];
static void _blueos_banner() {
    /* Print the primary kernel identification string */
    printk(WHITE, "%s\n", get_kernel_banner());

    /* Architecture-specific identification */
    printk(WHITE, "CPU: %s architecture detected.\n", BLUEOS_ARCH);

    /* Simulated boot arguments / Command line */
    printk(WHITE, "\nCommand line: BOOT_IMAGE=/boot/vmlinuz-%s root=UUID=mem-fs ro quiet\n", 
            UTS_RELEASE);
}

/**
 * print_boot_logs - Output early boot dmesg simulation.
 * * This provides the classic Linux 'timestamped' log look.
 */
static void print_boot_logs() {
    printk(WHITE, "[    0.000000] x86/fpu: Supporting XSAVE with 0x002 bits\n");
    printk(WHITE, "[    0.005000] BIOS-provided physical RAM map:\n");
    printk(WHITE, "[    0.005123]  BIOS-e820: [mem 0x0000000000000000-0x000000000009fbff] usable\n");
    printk(WHITE, "[    0.015842] ACPI: Core revision 20220210\n");
    printk(WHITE, "[    0.020000] Memory: 2048M/4096M available (16384K kernel code)\n");
    printk(WHITE, "[    0.032000] SLUB: Genslabs=2048, HWAlign=64, Order=0-3, MinObjects=0\n");
    printk(WHITE, "[    0.040000] VFS: Mounted root (ramfs filesystem) on /dev/ram0\n");
    printk(WHITE, "[    0.042000] devtmpfs: initialized and mounted\n");
}

/**
 * rest_init - Finalize kernel initialization and spawn the login shell.
 * * This follows the logic of the original rest_init() in linux/init/main.c.
 * It transition from kernel-space initialization to user-space interaction.
 */
static void rest_init() {

    
    printk(WHITE, "[    0.100000] Run /sbin/init as init process\n");
    printk(WHITE, "[    0.105000] Freeing unused kernel image memory: 2048K\n");

    /* Terminal/TTY identification banner */
    printk(WHITE, "\nBlueOS %s-generic tty1\n\n", UTS_RELEASE);
    
    /* Prompt handling for authentication */
    if (current_user_index == -1) {
        printk(WHITE, "blueos login: ");
    } else {
        printk(GREEN, "user@blueos");
        printk(WHITE, ":~$ ");
    }
}

void k_main(void)
{
    init_all();

    /* Phase 1: Hardware Init */
    system_state = SYSTEM_BOOTING;
    clear_screen();

    /* Phase 2: Banner and Core Subsystems */
    _blueos_banner();
    
    /* Phase 3: Subsystems initialization */
    fs_init();

    vfs_init();
    
    auth_init();
    current_user_index = -1; 

    task_init();
    mm_init();
    lru_init();
    profile_init(0x100000, 0x200000);
    
    
    

    system_state = SYSTEM_RUNNING;
    rest_init();
    
    while (1) {
        keyboard_handler();
    }
}