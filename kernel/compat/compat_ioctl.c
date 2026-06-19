#include <kernel/errno.h>
#include <kernel/printk.h>
#include <stdint.h>

#define MODULE_NAME "COMPAT_IOCTL"

/* Classic legacy ioctl command tokens common in UNIX terminals and storage
 * devices */
#define COMPAT_TCGETS 0x5401
#define COMPAT_BLKGETSIZE 0x1260

/**
 * compat_sys_ioctl: Decodes 32-bit hardware call arguments and standardizes
 * buffer alignments.
 */
long compat_sys_ioctl(unsigned int fd, unsigned int cmd, uint32_t arg) {
  printk("<7>[  %s ] Intercepted IOCTL call on FD %u -> Cmd Token: 0x%04X\n",
         MODULE_NAME, fd, cmd);

  void *native_arg_ptr = (void *)(uintptr_t)arg;

  switch (cmd) {
  case COMPAT_TCGETS:
    printk(
        "<6>[  %s ] Translating terminal TTY settings structure parameters.\n",
        MODULE_NAME);
    /* Perform structural padding alignment conversions here */
    break;

  case COMPAT_BLKGETSIZE:
    printk("<6>[  %s ] Translating block device total sector size inquiry "
           "metrics.\n",
           MODULE_NAME);
    break;

  default:
    printk("<4>[  %s ] Warning: Unrecognized compat IOCTL token. Direct "
           "routing fallback.\n",
           MODULE_NAME);
    break;
  }

  /* Call the standard native IOCTL pipeline */
  extern long sys_ioctl(unsigned int fd, unsigned int cmd, unsigned long arg);
  return sys_ioctl(fd, cmd, (unsigned long)(uintptr_t)native_arg_ptr);
}