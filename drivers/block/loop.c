#include <kernel/errno.h>
#include <kernel/printk.h>
#include <stddef.h>
#include <stdint.h>

#define MODULE_NAME "LOOP"
#define MAX_LOOP_DEVICES 8
#define LOOP_SECTOR_SIZE 512

typedef struct {
  int id;
  int associated_fd; /* Tied backend file handle index within the file system */
  uint64_t total_sectors;
  int is_configured;
} loop_device_t;

static loop_device_t g_loop_dev_table[MAX_LOOP_DEVICES];

/**
 * loop_set_fd: Binds an active file descriptor to a specific virtual block
 * device loop node.
 */
int loop_set_fd(int loop_id, int file_fd, uint64_t file_size) {
  if (loop_id < 0 || loop_id >= MAX_LOOP_DEVICES)
    return -EINVAL;
  if (g_loop_dev_table[loop_id].is_configured)
    return -EBUSY;

  g_loop_dev_table[loop_id].associated_fd = file_fd;
  g_loop_dev_table[loop_id].total_sectors = file_size / LOOP_SECTOR_SIZE;
  g_loop_dev_table[loop_id].is_configured = 1;

  printk("<6>[  %s  ] Loop node /dev/loop%d successfully bound to backend FD: "
         "%d (%u sectors)\n",
         MODULE_NAME, loop_id, file_fd,
         g_loop_dev_table[loop_id].total_sectors);

  return 0;
}

/**
 * loop_transfer_block: Performs sector mapping translations from block sector
 * layout to file offset parameters.
 */
int loop_transfer_block(int loop_id, int write_operation, uint64_t sector_num,
                        uint8_t *buffer, uint32_t count) {
  if (loop_id < 0 || loop_id >= MAX_LOOP_DEVICES)
    return -EINVAL;
  loop_device_t *dev = &g_loop_dev_table[loop_id];

  if (!dev->is_configured)
    return -ENXIO;
  if (sector_num + count > dev->total_sectors)
    return -EIO;

  uint64_t file_offset = sector_num * LOOP_SECTOR_SIZE;

  /* Interface bridges into your global POSIX file descriptor abstraction layer
   * modules */
  extern long sys_read(int fd, void *buf, size_t count);
  extern long sys_write(int fd, const void *buf, size_t count);

  /* Real systems loop would execute a sys_lseek here before issuing a
   * sequential raw read/write handle */
  if (write_operation) {
    printk("<7>[  %s  ] Loop%d redirecting block write offset: 0x%llX\n",
           MODULE_NAME, loop_id, file_offset);
    // sys_write(dev->associated_fd, buffer, count * LOOP_SECTOR_SIZE);
  } else {
    printk("<7>[  %s  ] Loop%d redirecting block read offset: 0x%llX\n",
           MODULE_NAME, loop_id, file_offset);
    // sys_read(dev->associated_fd, buffer, count * LOOP_SECTOR_SIZE);
  }

  return 0;
}

/**
 * loop_init: Sets structural setup configurations during storage engine boots.
 */
void loop_init(void) {
  for (int i = 0; i < MAX_LOOP_DEVICES; i++) {
    g_loop_dev_table[i].id = i;
    g_loop_dev_table[i].associated_fd = -1;
    g_loop_dev_table[i].total_sectors = 0;
    g_loop_dev_table[i].is_configured = 0;
  }
  printk(
      "<6>[  %s  ] Virtual loopback block devices mapping matrix registered.\n",
      MODULE_NAME);
}