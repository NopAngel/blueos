/*
 * BlueOS init/init_fnc.c
 *
 * Core initialization sequence for system drivers and subsystems.
 *
 * Copyright (C) 2024-2026  NopAngel
 */

#include <fs/fs.h>
#include <blueos/io.h>
#include <fs/vfs.h>
#include <idt.h>
#include <drivers/scsi.h>
#include <blueos/kvm.h>
#include <multiboot.h>
#include <hpet.h>
#include <blueos/printk.h>


extern uint32_t total_blocks;

/* --- External Linker Symbols --- */
extern uint32_t _end; 
extern int current_user_index;

/* --- Internal Prototypes --- */
static void boot_msg(const char* subsystem, const char* msg, int status);
#define panic(reason, ...) k_panic(__FILE__, __LINE__, reason, ##__VA_ARGS__)
/* --- DMA Buffer for Security Check --- */
uint8_t disk_buffer[512] __attribute__((aligned(4096)));

/**
 * boot_msg - Professional styled boot logging
 * Status: 0 = OK (Green), 1 = Warning (Yellow), 2 = Error (Red)
 */
static void boot_msg(const char* subsystem, const char* msg, int status) {
    printk(WHITE, "[ ");
    if (status == 0)      printk(GREEN, "  OK  ");
    else if (status == 1) printk(YELLOW, " WARN ");
    else                 printk(RED, " FAIL ");
    printk(WHITE, " ] %-12s: %s\n", subsystem, msg);
}
void init_all (unsigned int magic, struct multiboot_info *mbd)
{
    uint8_t hash_output[32];
    clear_screen(); 
    idt_init();
    boot_msg("CPU", "Interrupt Descriptor Table ready", 0);

    mm_init(mbd);
    uint32_t total_mb = mm_get_total() / (1024 * 1024);
    uint32_t free_mb = mm_get_free() / (1024 * 1024);

    if (total_mb == 0) {
        panic("Memory detection failure: 0 MB reported by BIOS/Multiboot.");
    }

    printk(GREEN, "[  OK  ] ");
    printk(WHITE, "Memory: %d MB total, %d MB free detected\n", total_mb, free_mb);

    boot_msg("PMM", "Physical Memory Manager active", 0);

    hdcdma_init(disk_buffer, 512);
    outb(0x1F7, 0xC8); 
    wait_for_disk();
    
    sha256_quick_hash(disk_buffer, 512, hash_output);
    if (hash_output[0] != 0x00) { 
        panic("SECURITY VIOLATION: Disk signature mismatch! Expected 0x00, got 0x%x", hash_output[0]);
    }
    boot_msg("SECURITY", "Kernel integrity verified via SHA-256", 0);

    int virt_ok = init_intel_vtx() || init_amd_svm();
    if (!virt_ok) {
        boot_msg("VIRT", "No hardware acceleration support found", 1);
    }
    vfs_init();
    fs_init();
    jfs_init();
    boot_msg("FS", "Journaling File Systems initialized", 0);


    isapnp_init();
    bcma_scan_bus();
    usbscan_init();
    
    if (find_wifi_card() == 0) {
        boot_msg("NETWORK", "No WiFi adapter detected", 1); // Warning
    } else {
        boot_msg("NETWORK", "Wireless card linked", 0);
    }

    //write_tss(5, 0x10, (uint32_t)kmalloc(4096) + 4096); 
    //flush_tss();
    // jump_to_user(test_user_function);
    scsi_init();
    auth_init();
    current_user_index = -1;
    pinctrl_init();
    leds_init();
    profile_init(0x100000, 0x200000);
    
    boot_msg("SYSTEM", "Runlevel 1 reached. Enabling Interrupts...", 0);
    printk(WHITE, "\n--------------------------------------------------\n");

    __asm__ volatile ("sti");
}