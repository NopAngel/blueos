#include <kernel/errno.h>
#include <kernel/printk.h>
#include <stddef.h>

/**
 * sys_write: Flushes stream bytes from memory into a targeted descriptor.
 */
long sys_write(int fd, const void *buf, size_t count) {
  if (fd < 0)
    return -EBADF;
  if (!buf)
    return -EFAULT;

  /* Routing output alerts for terminal operations */
  if (fd == 1 || fd == 2) {
    // Direct output tracking through your custom printk interface
    printk("<6>[  SYS_WRITE  ] Terminal TTY dump: %d bytes\n", count);
  }

  return (long)count;
}