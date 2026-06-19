#include <kernel/errno.h>
#include <kernel/printk.h>
#include <stddef.h>

/**
 * sys_mmap: Maps files or devices into memory pages.
 */
void *sys_mmap(void *addr, size_t length, int prot, int flags, int fd,
               long offset) {
  if (length == 0)
    return (void *)-EINVAL;

  printk("<6>[  SYS_MMAP   ] Allocation request at %p (%d bytes) with flags "
         "0x%X\n",
         addr, length, flags);
  (void)prot;
  (void)fd;
  (void)offset;

  /* Requesting memory block back from your kmalloc layout */
  extern void *kmalloc(uint32_t size);
  return kmalloc(length);
}