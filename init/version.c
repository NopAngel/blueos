/*
 * BlueOS/init/version.c
 *
 * Copyright (C) 2024-2026 Angel Nieto
 * Based on the original Linux version.c by Theodore Ts'o
 */

#include <kernel/printk.h>
#include <kernel/colors.h>
#include <generated/utsrelease.h>
#include <version.h>

/* The version of the compiler used to build the kernel */
#ifdef __clang__
    #define BLUEOS_COMPILER "Clang " __clang_version__
#elif defined(__GNUC__)
    #define BLUEOS_COMPILER "GCC " __VERSION__
#else
    #define BLUEOS_COMPILER "Unknown compiler"
#endif

/* * The banner that will be printed at boot time.
 * Using your printk format with color support.
 */
const char blueos_banner[] =
    "%s version %s (" BLUEOS_COMPILE_BY "@" UTS_VERSION ") "
    "(" BLUEOS_COMPILER ") %s\n";

/**
 * Displays the system banner to the console.
 * This is usually one of the first things called in k_main.
 */
void display_banner(void) {
    printk("--------------------------------------------------\n");
    printk(blueos_banner, "BlueOS", BLUEOS_VERSION, "SMP PREEMPT");
    printk("--------------------------------------------------\n");
}

/* * Weak definitions to allow the build system to inject
 * specific versioning at the last step if needed.
 */
const char blueos_release[] __attribute__((weak)) = BLUEOS_VERSION;
const char blueos_compile_by[] __attribute__((weak)) = BLUEOS_COMPILE_BY;
