#include <kernel/colors.h>
#include <kernel/errno.h>
#include <kernel/printk.h>
#include <stddef.h>
#include <stdint.h>

#define MODULE_NAME "RAMDISK"
#define RAMDISK_SECTOR_SIZE 512
#define RAMDISK_TOTAL_SECTORS                                                  \
  8192 /* 8192 * 512 bytes = 4MB storage disk size capacity */

static uint8_t *g_ramdisk_memory_pool = NULL;

/**
 * ramdisk_rw_sector: Copies data directly inside the system raw RAM pool
 * borders.
 */
int ramdisk_rw_sector(int write_op, uint64_t sector, uint8_t *buffer,
                      uint32_t sector_count) {
  if (sector + sector_count > RAMDISK_TOTAL_SECTORS)
    return -EIO;
  if (!buffer || !g_ramdisk_memory_pool)
    return -EFAULT;

  uint64_t byte_offset = sector * RAMDISK_SECTOR_SIZE;
  uint32_t transfer_bytes = sector_count * RAMDISK_SECTOR_SIZE;

  extern void *mm_memcpy(void *dest, const void *src, size_t n);

  if (write_op) {
    mm_memcpy(&g_ramdisk_memory_pool[byte_offset], buffer, transfer_bytes);
  } else {
    mm_memcpy(buffer, &g_ramdisk_memory_pool[byte_offset], transfer_bytes);
  }

  return 0;
}

/**
 * ramdisk_init: Allocates continuous raw page blocks inside core RAM structures
 * to establish /dev/ram0.
 */
void ramdisk_init(void) {
  boot_msg(MODULE_NAME,
           "Allocating memory buffers for initial system Ramdisk...", 0);

  /* Bound allocation handle directly into your kernel memory allocator routines
   */
  extern void *kmalloc(uint32_t size);
  g_ramdisk_memory_pool =
      (uint8_t *)kmalloc(RAMDISK_TOTAL_SECTORS * RAMDISK_SECTOR_SIZE);

  if (!g_ramdisk_memory_pool) {
    printk("<3>[  %s  ] Error: Failed to secure continuous system memory "
           "allocations.\n",
           MODULE_NAME);
    return;
  }

  /* Zero out the entire disk space to provide clear block definitions */
  for (uint32_t i = 0; i < (RAMDISK_TOTAL_SECTORS * RAMDISK_SECTOR_SIZE); i++) {
    g_ramdisk_memory_pool[i] = 0;
  }

  printk("<6>[  %s  ] Registered block disk /dev/ram0 successfully [Size: 4MB, "
         "Sectors: %d]\n",
         MODULE_NAME, RAMDISK_TOTAL_SECTORS);
}