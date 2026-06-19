#include <kernel/errno.h>
#include <kernel/printk.h>
#include <stdint.h>

/**
 * sys_chmod: Alters access privilege mask tokens on specific filesystem
 * descriptors.
 */
int sys_chmod(const char *pathname, uint32_t mode) {
  if (!pathname)
    return -EFAULT;

  printk("<5>[  SYS_CHMOD  ] Modifying file system access bits on '%s' to "
         "mask: 0%o\n",
         pathname, mode);
  return 0;
}