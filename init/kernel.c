/*
 * BlueOS arch/i386/kernel.c
 *
 * Core Kernel Entry Point. This file orchestrates the high-level 
 * initialization sequence after the bootloader handoff.
 *
 * Copyright (C) 2024-2026  NopAngel <angelgabrielnieto@outlook.com>
 *
 * This source code is licensed under the MIT License.
 */

#include <blueos/colors.h>
#include <blueos/printk.h>
#include <blueos/ports.h>
#include <blueos/panic.h>
#include <blueos/task.h>
#include <blueos/rtc.h>
#include <drivers/keyboard.h>
#include <init_fnc.h>
#include <version.h>
#include <multiboot.h>



#define KERNEL_STACK_SIZE       4096
uint8_t kernel_stack_buffer[KERNEL_STACK_SIZE] __attribute__ ((aligned(16)));
uint32_t get_kernel_stack_top() {
    return (uint32_t)kernel_stack_buffer + KERNEL_STACK_SIZE;
}

enum system_states {
    SYSTEM_BOOTING,
    SYSTEM_RUNNING,
    SYSTEM_PANIC
} system_state;

void test_audio_driver() {
    printk(YELLOW, "Testing Sound Driver... ");
    
    for (int i = 0; i < 3; i++) {
        play_sound(880); // Nota La alta
        for(volatile int d = 0; d < 20000000; d++); 
        
        play_sound(440); // Nota La normal
        for(volatile int d = 0; d < 20000000; d++);
    }
    
    nosound();
    printk(GREEN, "OK\n");
}



static void _blueos_banner() {
    clear_screen();

    // The "Blue" Shell Logo
    printk(CYAN,  "  ____  _            \n");
    printk(CYAN,  " | __ )| |_   _  ___ "); printk(WHITE, "   Kernel: "); printk(GRAY, "%s\n", UTS_RELEASE);
    printk(CYAN,  " |  _ \\| | | | |/ _ \\"); printk(WHITE, "   Arch:   "); printk(GRAY, "%s\n", BLUEOS_ARCH);
    printk(CYAN,  " | |_) | | |_| |  __/\n");
    printk(CYAN,  " |____/|_|\\__,_|\\___|"); printk(WHITE, "   UTS VERSION:   "); printk(GRAY, "%s\n", UTS_VERSION);
    
    printk(CYAN, "\n --------------------------------------------------------------\n\n");

    display_system_palette();
}


/**
 * format_rtc_val - Ensures two-digit formatting for time/date
 */
static void format_rtc_val(int val) {
    if (val < 10) printk(CYAN, "0");
    printk(CYAN, "%d", val);
}

/**
 * rest_init - Final initialization before jumping to User Space
 */
static void rest_init() {
    int sec, min, hour, day, month, year;

    /* Synchronize with CMOS Real Time Clock */
    read_rtc(&sec, &min, &hour, &day, &month, &year);

    printk(WHITE, "\n[ System Clock Sync ]\n");
    printk(GRAY, " Date: "); 
    format_rtc_val(day);   printk(GRAY, "/"); 
    format_rtc_val(month); printk(GRAY, "/20"); 
    format_rtc_val(year);
    
    printk(GRAY, " | Time: "); 
    format_rtc_val(hour);  printk(GRAY, ":"); 
    format_rtc_val(min);   printk(GRAY, ":"); 
    format_rtc_val(sec);   printk(WHITE, "\n\n");

    printk(WHITE, "BlueOS %s-generic tty1\n", UTS_RELEASE);
    
}

/**
 * k_main - Kernel Entry Point (The Master Orchestrator)
 */
void k_main(unsigned int magic, multiboot_info_t* mbi) {
    uint32_t stack_top = get_kernel_stack_top();
    __asm__ __volatile__ ("movl %0, %%esp" : : "r"(stack_top));
    system_state = SYSTEM_BOOTING;
    init_all(magic, mbi); 


    _blueos_banner();
    system_state = SYSTEM_RUNNING;
    rest_init();

    while (1) {
        keyboard_handler();
        update_battery_status();
    }
}