/*
 * BlueOS init/init_fnc.c
 *
 * Core initialization sequence for system drivers and subsystems.
 *
 * Copyright (C) 2024-2026  NopAngel
 */

#include <stdint.h>
#include <stdbool.h>
#include <fs/fs.h>
#include <kernel/io.h>
#include <fs/vfs.h>
#include <arch/i386/idt.h>
#include <drivers/scsi.h>
#include <kernel/kvm.h>
#include <multiboot.h>
#include <hpet.h>
#include <kernel/printk.h>

#define RTL_VENDOR_ID 0x10EC
#define RTL_DEVICE_ID 0x8139

extern uint32_t total_blocks;
extern uint32_t _end;
extern int current_user_index;

const char *readme_lol = "  BLUEOS  \nExplanation:\n It's a kernel inspired by GNU/Linux and Unix.\n\n How to use it?\n\nIt works similarly to Bash. Up and down arrows: to see the commands\nTab: to see the continuation of the command\nDouble Tab: for when there are two identical commands and you need to see the 'similar'\n\nThe commands:\n You can use 'help' to see the list of commands\n\nCreated by: NopAngel   Repo: github.com/NopAngel/blueos";

static void x86_wallclock_init(void);
void i386_init_noop(void);
void idt_init(void);
void fs_init(void);
extern void mm_init(struct multiboot_info *mbi);
void detect_hypervisor(void);
bool i386_is_guest(void);
const char* i386_hyper_name(void);
void i386_memory_prepare(struct multiboot_info* mbi);
static void i386_mem_init(struct multiboot_info *mbd) {
    if (mbd) {
        mm_init(mbd);
    }
}

struct i386_hyper_ops {
    void (*init_platform)(void);
    bool (*is_guest)(void);
    const char* (*get_name)(void);
};

struct i386_init_ops {
    void (*resources_setup)(struct multiboot_info *);
    void (*arch_setup)(void);
    void (*mem_init)(struct multiboot_info *);
    void (*fs_init)(void);
    struct i386_hyper_ops hyper;
};

void i386_init_noop(void) { }

/* --- Global Init Table --- */
struct i386_init_ops blueos_init = {
    .resources_setup = i386_memory_prepare,
    .arch_setup = idt_init,
    .mem_init   = mm_init,
    .fs_init    = fs_init,

    .hyper = {
        .init_platform = detect_hypervisor,
        .is_guest      = i386_is_guest,
        .get_name      = i386_hyper_name,
    }
};

static uint8_t disk_buffer[512] __attribute__((aligned(4096)));

/**
 * boot_msg - Professional styled boot logging
 * Status: 0 = OK, 1 = WARN, 2 = FAIL
 */
static void boot_msg(const char* subsystem, const char* msg, int status) {
    switch (status) {
        case 0:
            pr_info("[  OK  ] %s-12s: %s\n", subsystem, msg);
            break;
        case 1:
            pr_warn("[ WARN ] %s-12s: %s\n", subsystem, msg);
            break;
        case 2:
            pr_err( "[ FAIL ] %s-12s: %s\n", subsystem, msg);
            break;
    }
}

/**
 * verify_kernel_integrity - SHA-256 Check of Boot Sector
 */
static void verify_kernel_integrity(void) {
    uint8_t hash_output[32];
    hdcdma_init(disk_buffer, 512);
    outb(0x1F7, 0xC8); // READ DMA command
    wait_for_disk();

    sha256_quick_hash(disk_buffer, 512, hash_output);
    if (hash_output[0] != 0x00) {
        k_panic(__FILE__, __LINE__, "SECURITY VIOLATION: Disk signature mismatch!");
    }
    boot_msg("SECURITY", "Kernel integrity verified via SHA-256", 0);
}

/**
 * init_all - Main kernel entry sequence
 */
void init_all(unsigned int magic, struct multiboot_info *mbd) {
    clear_screen();

    blueos_init.arch_setup();
    boot_msg("CPU", "IDT and CPU descriptors ready", 0);

    blueos_init.mem_init(mbd);
    uint32_t total_mb = mm_get_total() / (1024 * 1024);
    if (total_mb == 0) {
        k_panic(__FILE__, __LINE__, "Memory detection failure: 0 MB reported.");
    }
    boot_msg("PMM", "Physical Memory Manager active", 0);
    printk(WHITE, "             Detected: %d MB total RAM\n", total_mb);

    verify_kernel_integrity();

    if (!init_intel_vtx() && !init_amd_svm()) {
        boot_msg("VIRT", "No hardware acceleration (VMX/SVM) available", 1);
    }



    const char* cmdline = (const char*)mbd->cmdline;

    if (cmdline_find_option_bool(cmdline, "quiet")) {
        // ...
    }

    char log_level[4];
    if (cmdline_find_option(cmdline, "loglevel", log_level, sizeof(log_level)) > 0) {
        // ..
    }

    boot_msg("CMDLINE", "Kernel parameters parsed", 0 );

    blueos_init.hyper.init_platform();


    if (blueos_init.hyper.is_guest()) {
        boot_msg("HYPER", "Virtual environment detected", 0);
        printk(CYAN, "             Platform: %s\n", blueos_init.hyper.get_name());
    } else {
        boot_msg("HYPER", "Running on Bare Metal", 0);
    }

    vfs_init();
    blueos_init.fs_init();
    //jfs_init();
    boot_msg("FS", "VFS and Journaling FS (JFS) ready", 0);

    isapnp_init();
    bcma_scan_bus();
    usbscan_init();
    scsi_init();

    uint32_t net_dev = pci_find_device(RTL_VENDOR_ID, RTL_DEVICE_ID);
    if (net_dev != 0xFFFFFFFF) {
        uint8_t bus = (net_dev >> 16) & 0xFF;
        uint8_t slot = (net_dev >> 8) & 0xFF;
        rtl8139_init(bus, slot);
        boot_msg("NET", "RTL8139 controller initialized", 0);
    }

    auth_init();
    current_user_index = -1;
    pinctrl_init();
    leds_init();
    profile_init(0x100000, 0x200000);

    boot_msg("SYSTEM", "Runlevel 1 reached. Enabling IRQs", 0);
    printk(WHITE, "--------------------------------------------------\n");
    touch("ReadMe.md", readme_lol);


    __asm__ volatile ("sti");

}
