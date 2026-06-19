#include <kernel/errno.h>
#include <kernel/printk.h>
#include <stddef.h>
#include <stdint.h>

/**
 * sys_open: Allocates a new File Descriptor (FD) and binds it to a VFS node.
 */
int sys_open(const char *pathname, int flags, int mode) {
  if (!pathname)
    return -EFAULT;

  printk("<6>[  SYS_OPEN  ] Opening path '%s' with flags: 0x%X, mode: 0x%X\n",
         pathname, flags, mode);

  /* Mocking VFS descriptor allocation index */
  int allocated_fd =
      3; /* Standard user-space FDs start after stdin/stdout/stderr */

  return allocated_fd;
}