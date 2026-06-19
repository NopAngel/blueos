#include <kernel/errno.h>
#include <kernel/printk.h>

struct posix_stat {
  uint32_t st_dev;
  uint32_t st_ino;
  uint32_t st_mode;
  uint32_t st_size;
};

/**
 * sys_stat: Retrieves detailed operational metadata attributes from a target
 * file path.
 */
int sys_stat(const char *pathname, struct posix_stat *statbuf) {
  if (!pathname)
    return -EFAULT;
  if (!statbuf)
    return -EFAULT;

  printk("<6>[  SYS_STAT   ] Inspecting attributes for object node: '%s'\n",
         pathname);

  /* Populating standard dummy metrics structure */
  statbuf->st_dev = 1;
  statbuf->st_ino = 101;
  statbuf->st_mode = 00644; /* Standard read/write profile */
  statbuf->st_size = 4096;

  return 0;
}