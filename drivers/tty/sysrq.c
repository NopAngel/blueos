#include <kernel/colors.h>
#include <kernel/errno.h>
#include <kernel/panic.h>
#include <kernel/printk.h>
#include <stdint.h>

#define MODULE_NAME "TTY_SYSRQ"

/**
 * sysrq_handle_crash: Forces an immediate kernel panic for crash dump testing.
 */
static void sysrq_handle_crash(void) {
  printk("<3>[  %s  ] SysRq: Triggering intentional Kernel Panic for debug "
         "analysis...\n",
         MODULE_NAME);
  k_panic(4, "SysRq: User-triggered core crash dump sequence.");
}

/**
 * sysrq_handle_sync: Forces the Virtual File System to flush dirty buffers to
 * storage.
 */
static void sysrq_handle_sync(void) {
  printk("<5>[  %s  ] SysRq: Emergency Emergency! Flushing VFS dirty blocks "
         "buffers to disk sync...\n",
         MODULE_NAME);
  /* Extern hook into your VFS sync code:
   * extern void vfs_sync_all(void);
   * vfs_sync_all();
   */
}

/**
 * handle_sysrq: Entry point called directly by the keyboard interrupt handler
 * routine.
 */
void handle_sysrq(char key) {
  printk("<4>[  %s  ] SysRq: Intercepted Magic System Request key combination: "
         "'%c'\n",
         MODULE_NAME, key);

  switch (key) {
  case 'c': /* Crash the system */
    sysrq_handle_crash();
    break;

  case 's': /* Sync filesystems */
    sysrq_handle_sync();
    break;

  case 'p': /* Dump current registers CPU state */
    printk("<6>[  %s  ] SysRq: Dumping active CPU instruction registers and "
           "task context frames.\n",
           MODULE_NAME);
    /* Route directly to your profile.c or arch scheduler dumps */
    break;

  default:
    printk("<4>[  %s  ] SysRq: Unknown command token. (Available: 'c'=Panic, "
           "'s'=Sync, 'p'=Regs)\n",
           MODULE_NAME);
    break;
  }
}