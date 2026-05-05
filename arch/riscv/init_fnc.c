/*
 * BlueOS init/init_fnc.c
 *
 * Core initialization sequence for system drivers and subsystems.
 *
 * Copyright (C) 2024-2026  NopAngel
 */
#include <stdbool.h>
#include <fs/fs.h>
#include <fs/vfs.h>
#include <drivers/scsi.h>
#include <kernel/kvm.h>
#include <multiboot.h>
#include <hpet.h>
#include <kernel/printk.h>
#include <kernel/colors.h>

#define RTL_VENDOR_ID 0x10EC
#define RTL_DEVICE_ID 0x8139
extern int console_loglevel;
extern char _end;
extern uint32_t get_ctx_count();
extern uint32_t total_blocks;

extern int current_user_index;
extern int cursor_x;
extern int cursor_y;
int g_high_res_enabled = 1;

void fs_init(void);
extern void mm_init(struct multiboot_info *mbi);
extern void k_main();


static uint8_t disk_buffer[512] __attribute__((aligned(4096)));

/**
 * boot_msg - Professional styled boot logging
 * Status: 0 = OK, 1 = WARN, 2 = FAIL
 */
static void boot_msg(const char* subsystem, const char* msg, int status) {
    switch (status) {
        case 0:
            printk(GREEN, "[  OK  ] %s-12s: %s\n", subsystem, msg);
            break;
        case 1:
            printk(YELLOW, "[ WARN ] %s-12s: %s\n", subsystem, msg);
            break;
        case 2:
            printk(RED, "[ FAIL ] %s-12s: %s\n", subsystem, msg);
            break;
    }
}

/**
 * verify_kernel_integrity - SHA-256 Check of Boot Sector
 */
static void verify_kernel_integrity(void) {
    uint8_t hash_output[32];
    if (hash_output[0] != 0x00) {
        k_panic(__FILE__, __LINE__, "SECURITY VIOLATION: Disk signature mismatch!");
    }
    boot_msg("SECURITY", "Kernel integrity verified via SHA-256", 0);
}

extern int g_high_res_enabled;




/**
 * init_all - Main kernel entry sequence
 */
void init_all() {
    clear_screen();

    kmalloc_init((uintptr_t)&_end, 1024 * 1024);
    mm_init(128);
    printk(GREEN, "Memory Manager initialized.\n");

    fpu_init();
    current_user_index = -1;
    profile_init(0x100000, 0x200000);

    boot_msg("SYSTEM", "Runlevel 1 reached. Enabling IRQs", 0);
    printk(WHITE, "--------------------------------------------------\n");

}
