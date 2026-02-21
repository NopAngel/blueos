/*
 * BlueOS / fs / sysfs.c
 *
 * Copyright (C) 2024-2026  NopAngel <angelgabrielnieto@outlook.com>
 *
 * Description:
 * Implementation of the /sys pseudo-filesystem (sysfs).
 * This allows kernel subsystems to export internal state to user-space.
 */

#include <include/sysfs.h>
#include <include/fs/vfs.h>
#include <include/lib/string.h>
#include <include/version.h>
#include <include/printk.h>
#include <include/colors.h>

/**
 * sysfs_read_version() - Read handler for /sys/kernel/version.
 * @buffer: The kernel buffer where the data will be copied.
 *
 * Return: Byte count of the generated string.
 */
int sysfs_read_version(char *buffer) {
    strcpy(buffer, "BlueOS Kernel v");
    strcat(buffer, UTS_RELEASE);
    strcat(buffer, "\n");
    return strlen(buffer);
}

/**
 * sysfs_init() - Mount and populate the /sys hierarchy.
 *
 * This function is called during the late boot phase to expose 
 * kernel data to the VFS.
 */
void sysfs_init() {
    /* Create directory structure */
    vfs_register_node("/sys", 1, 0);         /* Directory */
    vfs_register_node("/sys/kernel", 1, 0);  /* Sub-Directory */
    vfs_register_node("/sys/kernel/version", 0, sysfs_read_version); /* File's */
    
    printk(WHITE, "[    0.080000] sysfs: initialized and mounted on /sys\n");
}