#include <kernel/errno.h>
#include <kernel/printk.h>
#include <stddef.h>

/**
 * sys_read: Fetches bytes from an open File Descriptor into a user buffer.
 */
long sys_read(int fd, void *buf, size_t count) {
  if (fd < 0)
    return -EBADF;
  if (!buf)
    return -EFAULT;

  printk("<7>[  SYS_READ  ] Requesting %d bytes from file descriptor (%d)\n",
         count, fd);

  /* Simulated read feedback loop */
  return (long)count;
}