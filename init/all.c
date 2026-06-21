/*
 * BlueOS init/all.c
 * Core initialization sequence for system drivers and subsystems.
 * Professional & Multi-arch version.
 * Strongly inspired by OpenBSD security lifecycle principles.
 * * Pure C, strictly in English.
 */

#include <arch/x86/timer.h>
#include <auth.h>
#include <drivers/cdrom.h>
#include <drivers/disk.h>
#include <drivers/hypervisor.h>
#include <drivers/keyboard.h>
#include <drivers/leds.h>
#include <drivers/pci.h>
#include <drivers/pinctrl.h>
#include <drivers/power.h>
#include <drivers/tmpfs.h>
#include <drivers/tty.h>
#include <drivers/usb_core.h>
#include <drivers/vt100.h>
#include <fs/ext2.h>
#include <fs/fs.h>
#include <fs/initramfs.h>
#include <fs/vfs.h>
#include <fs/xfs.h>
#include <kernel/arch.h> 
#include <kernel/panic.h>
#include <kernel/printk.h>
#include <kernel/ebpf.h> /* Native secure eBPF subsystem */
#include <mm/memory.h>
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <multiboot.h>
#include <profile.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys.h>
#include <version.h>

/* External structures and symbols */
extern vfs_ops_t xfs_ops;
extern struct vfs_node *g_root_node;
extern int current_user_index;


extern int g_gui_enabled;
extern void scsi_init(void);
extern void isapnp_init(void);
extern void usbscan_init(void);
extern void bcma_scan_bus(void);
extern bool arch_is_guest(void);
extern void arch_early_init(void);
extern const char *arch_get_hypervisor_name(void);
extern void init_tss(uint16_t ss, uint32_t esp);
extern void ata_read_sector(uint32_t lba, uint8_t *buffer);
extern int vfs_create_binary(const char *name, void *buffer, uint32_t size);

/* Generic kernel layout boundaries */
extern uint8_t _kernel_start;
extern uint8_t _kernel_end;

const char *readme_lol = "  BLUEOS  \nExplanation:\n It's a kernel inspired by "
                         "GNU/Linux and Unix.\n\nCreated by: NopAngel";

/**
 * tty_putchar_wrapper - Redirects system character output to the VT100 console engine
 */
void tty_putchar_wrapper(char c) {
    vt100_putc(c);
}

/**
 * _has_installed_ext2 - Checks for valid ext2 signatures on the main block device
 */
static bool _has_installed_ext2(void) {
    uint8_t buffer[512];
    disk_read(2050, buffer, 1);
    ext2_superblock_t *sb = (ext2_superblock_t *)buffer;
    return sb->s_magic == EXT2_MAGIC;
}

/**
 * init_all - Main kernel infrastructure initialization lifecycle
 */
void init_all(unsigned int magic, void *arch_data) {
    printk("\n\n\n");
    
    /* Phase 1: Architecture Lower-Half & Memory Topologies Initialization */
    mm_init(arch_data);
    arch_early_init(); /* Single, safe idempotent entry execution point */
    
    uint64_t real_mem = mm.total_memory;
    uint64_t avail_mem = mm_get_free_memory();
    uint32_t real_mb = (uint32_t)(real_mem / (1024 * 1024));
    uint32_t avail_mb = (uint32_t)(avail_mem / (1024 * 1024));

    boot_msg("PMM", "Physical Memory Manager active", 0);
    printk("<6> MEM: %u MB RAM detected | %u MB free for processes\n", real_mb, avail_mb);

    /* Phase 2: Execution Sandbox & Virtual Terminals Initialization */
    vfs_init();
    xfs_init();   
    vt100_init();
    tty_init(tty_putchar_wrapper);
    boot_msg("TTY", "Virtual console ready", 0);
    boot_msg("VT100", "Terminal driver initialized", 0);

    /* Phase 3: Environment Detection */
    if (arch_is_guest()) {
        boot_msg("HYPER", "Virtual environment detected", 0);
        printk("\033[36m             Platform: %s\033[0m\n", arch_get_hypervisor_name());
    } else {
        boot_msg("HYPER", "Running on Bare Metal", 0);
    }

    /* Phase 4: Bootloader Metadata Parsing & Ramdisk Unpacking */
    multiboot_info_t *mbi = (multiboot_info_t *)arch_data;

    if (mbi->flags & (1 << 1)) {
        uint8_t drive = (mbi->boot_device >> 24) & 0xFF;
        const char *boot_type = "Unknown";

        if (drive >= 0xE0 && drive <= 0xEF)      boot_type = "CD/DVD";
        else if (drive >= 0x80 && drive <= 0x8F) boot_type = "Hard Disk";
        else if (drive <= 0x7F)                  boot_type = "Floppy";

        printk("<6> BOOT: %s (Drive: 0x%x)\n", boot_type, drive);
    } else {
        printk("<4> WARN: The boot device could not be determined\n");
    }

    if (g_root_node) {
        g_root_node->ops = &xfs_ops;
    }
    
    if (magic == MULTIBOOT_BOOTLOADER_MAGIC && (mbi->flags & MULTIBOOT_INFO_MODS)) {
        multiboot_module_t *mod = (multiboot_module_t *)mbi->mods_addr;
        initramfs_parse(mod->mod_start, mod->mod_end);
    }
    
    current_user_index = -1;
    boot_msg("CPU", "Architecture descriptors ready", 0);

    /* Initialize Task State Segment (TSS) for Ring 3 contexts switching */
    init_tss(0x10, 0x90000);

    /* Phase 5: Interrupt Control, Authentication & Secure Runtime Initialization */
    arch_enable_interrupts();
    boot_msg("IRQ", "Hardware interrupts enabled", 0);
    
    auth_init();
    boot_msg("AUTH", "Authentication subsystem hardened and ready", 0);

    /* Native eBPF Kernel Engine deployment (OpenBSD secure execution style) */
    ebpf_init();
    boot_msg("EBPF", "In-kernel secure execution engine active", 0);

    /* Phase 6: Legacy Bus Systems, Peripheral Scanning & PCI Discovery */
    isapnp_init();
    boot_msg("ISAPNP", "ISA Plug and Play subsystem loaded", 0);
    
    bcma_scan_bus();
    boot_msg("BCMA", "BCMA bus scanned", 0);
    
    usb_core_init();
    scsi_init();
    boot_msg("SCSI", "SCSI subsystem initialized", 0);
    
    pci_scan_bus();
    boot_msg("PCI", "PCI bus scan completed", 0);

    /* Phase 7: Pin Multiplexing, Power, Metrics Tracking & Post-Boot Actions */
    pinctrl_init();
    boot_msg("PINCTL", "Pin control subsystem ready", 0);
    
    
    if(g_gui_enabled == 1) {
        vfs_register_fb0(arch_data);
        boot_msg("FB", "Framebuffer Inited Success!", 0);
    }


    leds_init();
    boot_msg("LEDS", "LED subsystem ready", 0);
    
    profile_init(0x100000, 0x200000);
    boot_msg("PROFILE", "Profiler tracking active", 0);

    sysinit_run();
    boot_msg("SYS", "All kernel components initialized successfully", 0);

    /* Phase 8: Hard Storage Inventory Enlistment */
    printk("\nAvailable disks:\n");
    if (disk_count == 0) {
        printk("  none\n");
        boot_msg("STORAGE", "No disks detected", 1);
    } else {
        boot_msg("STORAGE", "Disk inventory ready", 0);
    }

    for (int i = 0; i < disk_count; i++) {
        printk("  [%d] %s\n", i, system_disks[i].name);
    }

    printk("Booting storage stack...\n");
    printk("Done.\n\n");

    printk("real memory    = %u (%u MB)\n", (uint32_t)real_mem, real_mb);
    printk("avail memory   = %u (%u MB)\n", (uint32_t)avail_mem, avail_mb);

    printk("<6> STORAGE: %d drive(s) localized on PCI bus.\n", disk_count);
    for (int i = 0; i < disk_count; i++) {
        disk_info_t *disk = &system_disks[i];
        printk("  [%d] %s: %s / %s / %s\n", i, disk->name, disk->transport,
               disk->media_type, disk->partition_style);
        for (int p = 0; p < disk->partition_count; p++) {
            partition_info_t *part = &disk->partitions[p];
            printk("      %s: %s %s start=%u sectors=%u\n", part->name,
                   part->active ? "ACTIVE" : "INACTIVE", part->type, part->start_lba,
                   part->sectors);
        }
    }
    printk("\n");

    /* Phase 9: Final Initrd Deployment Structures Parsing */
    if (mbi->flags & (1 << 3)) { 
        multiboot_module_t *mod = (multiboot_module_t *)mbi->mods_addr;
        for (uint32_t i = 0; i < mbi->mods_count; i++) {
            char *mod_name = (char *)mod[i].string;
            if (mod_name) {
                boot_msg("INITRD", mod_name, 0);
                vfs_create_binary(mod_name, (void *)mod[i].mod_start,
                                  mod[i].mod_end - mod[i].mod_start);
            }
        }
    }
}