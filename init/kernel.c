/*
 * BlueOS arch/kernel.c
 *
 * Copyright (C) 2024-2026  NopAngel <angelgabrielnieto@outlook.com>
 *
 */


#include <blueos/colors.h>
#include <blueos/printk.h>
#include <blueos/ports.h>
#include <hlec.h>
#include <blueos/panic.h>
#include <init_fnc.h>
#include <drivers/keyboard.h>
#include <fs/vfs.h>
#include <fs/fs.h>
#include <version.h>
#include <interrupts.h>
#include <multilru.h>
#include <profile.h>
#include <auth.h>
#include <blueos/task.h>
#include <multiboot.h>
#include <sysfs.h>
#include <elf.h>
#include <sysctl.h>
#include <drivers/rtc.h>
#include <mm/pmm.h>
#include <kernel/module.h>
#include <blueos/kvm.h>


extern uint64_t end;
uint64_t kernel_end_address; 

extern uint32_t used_blocks;
extern uint32_t total_blocks;
extern uint8_t _binary_hello_elf_start[];
typedef void (*entry_point)();
enum system_states {
    SYSTEM_BOOTING,
    SYSTEM_RUNNING,
    SYSTEM_PANIC
} system_state;

page_t system_page;
page_t user_page;
extern module_t __this_module;
extern void _DRIVER_PS2_Keyboard();

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


void print_rtc_formatted(int val) {
    if (val < 10) printk(CYAN, "0");
    printk(CYAN, "%d", val);
}


static void rest_init() {

    
    printk(WHITE, "[    0.100000] Run /sbin/init as init process\n");
    printk(WHITE, "[    0.105000] Freeing unused kernel image memory: 2048K\n");

    int sec, min, hour, day, month, year;
    read_rtc(&sec, &min, &hour, &day, &month, &year);

    printk(WHITE, "info: ");
    printk(GRAY, "Initialized RTC (24h mode, no daylight saving)\n");

    printk(WHITE, "info: ");
    printk(GRAY, "Current time: ");

    print_rtc_formatted(day);   printk(CYAN, "/");
    print_rtc_formatted(month); printk(CYAN, "/20");
    print_rtc_formatted(year);  printk(CYAN, " ");
    print_rtc_formatted(hour);  printk(CYAN, ":");
    print_rtc_formatted(min);   printk(CYAN, ":");
    print_rtc_formatted(sec);   printk(CYAN, "\n");
    printk(WHITE, "\nBlueOS %s-generic tty1\n\n", UTS_RELEASE);
    
    if (current_user_index == -1) {
        printk(WHITE, "\nblueos login: ");
    } else {
        printk(GREEN, "user@blueos");
        printk(WHITE, ":~$ ");
    }
}




void k_main (unsigned int magic, multiboot_info_t* mbi)
{
    init_all(magic, mbi); 


    system_state = SYSTEM_BOOTING;
    clear_screen();


    
    _blueos_banner();
    print_boot_logs(); 
    



    system_state = SYSTEM_RUNNING;
    rest_init();


    while (1)
    {
        keyboard_handler();
        update_battery_status();
    }
}