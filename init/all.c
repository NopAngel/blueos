/*
 * BlueOS init/init_fnc.c
 * Core initialization sequence for system drivers and subsystems.
 * Professional & Multi-arch version.
 */

#include <stdint.h>
#include <stdbool.h>
#include <kernel/arch.h>      // Universal arch functions
#include <kernel/printk.h>
#include <kernel/panic.h>
#include <mm/memory.h>
#include <fs/vfs.h>
#include <fs/fs.h>
#include <auth.h>
#include <drivers/pinctrl.h>
#include <drivers/leds.h>
#include <drivers/keyboard.h>
#include <drivers/vt220.h>
#include <drivers/tty.h>
#include <profile.h>

extern void scsi_init();
extern void isapnp_init();
extern void usbscan_init();
extern void bcma_scan_bus();
extern bool arch_is_guest(void);
extern void arch_early_init(void);
extern const char* arch_get_hypervisor_name(void);
extern int current_user_index;
const char *readme_lol = "  BLUEOS  \nExplanation:\n It's a kernel inspired by GNU/Linux and Unix.\n\nCreated by: NopAngel";

/**
 * boot_msg - Professional styled boot logging
 * Status: 0 = OK, 1 = WARN, 2 = FAIL
 */
static void boot_msg(const char* subsystem, const char* msg, int status) {
    switch (status) {
        case 0: pr_info("[  OK  ] %-12s: %s\n", subsystem, msg); break;
        case 1: pr_warn("[ WARN ] %-12s: %s\n", subsystem, msg); break;
        case 2: pr_err( "[ FAIL ] %-12s: %s\n", subsystem, msg); break;
    }
}

void tty_putchar_wrapper(char c) {
    arch_put_char(c, WHITE);
}


/**
 * init_all - Main kernel entry sequence 
 */
void init_all(void* arch_data) {
    printk(WHITE, "\n\n\n");
    mm_init(arch_data);

    current_user_index = -1;
    arch_early_init(); 
    boot_msg("CPU", "Architecture descriptors ready", 0);
    vt220_init();
    uint32_t total_mb = mm_get_total() / (1024 * 1024);
    boot_msg("PMM", "Physical Memory Manager active", 0);
    //printk(WHITE, "             Detected: %d MB total RAM\n", total_mb);
    tty_init(tty_putchar_wrapper);
    if (arch_is_guest()) {
        boot_msg("HYPER", "Virtual environment detected", 0);
        printk(CYAN, "             Platform: %s\n", arch_get_hypervisor_name());
    } else {
        boot_msg("HYPER", "Running on Bare Metal", 0);
    }

    vfs_init();
    fs_init();
    boot_msg("FS", "VFS and RootFS ready", 0);


    isapnp_init();
    bcma_scan_bus();
    usbscan_init();
    scsi_init();

    auth_init();
    add_user("root", "123");

    pinctrl_init();
    leds_init();
    profile_init(0x100000, 0x200000);

    printk(WHITE, "\n\n\n"); 
    // touch("ReadMe.md", readme_lol); 
    arch_enable_interrupts();
}
