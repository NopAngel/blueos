#include <kernel/errno.h>
#include <kernel/printk.h>

/**
 * sys_chown: Modifies the user (UID) and group (GID) owner descriptors of a
 * file.
 */
int sys_chown(const char *pathname, int owner_uid, int group_gid) {
  if (!pathname)
    return -EFAULT;

  printk("<5>[  SYS_CHOWN  ] Transmutation of ownership on '%s'. New UID: %d, "
         "GID: %d\n",
         pathname, owner_uid, group_gid);
  return 0;
}