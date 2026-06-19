#include <kernel/errno.h>
#include <kernel/ioctl.h>
#include <kernel/process.h>
#include <kernel/sched.h>
#include <lib/string.h>
#include <mm/memory.h>
#include <stdint.h>

/* Binder Protocol Definitions */

#define BINDER_WRITE_READ _IOWR('b', 1, struct binder_write_read)
#define BINDER_SET_MAX_THREADS _IOW('b', 2, uint32_t)
struct binder_write_read {
  uintptr_t write_buffer;
  uintptr_t read_buffer;
  uint32_t write_size;
  uint32_t read_size;
};

/* Binder Transaction Data - The core of the IPC */
struct binder_transaction_data {
  uint32_t handle;       // Target descriptor
  uint32_t code;         // RPC code (method to call)
  uint32_t flags;        // Transaction flags
  uintptr_t data_buffer; // Pointer to the actual data
  uint32_t data_size;    // Size of the data
};

/**
 * binder_transaction - Handles the delivery of IPC messages.
 * In a real kernel, this would locate the target process and
 * copy the data to its shared binder memory space.
 */
static int binder_transaction(struct task_struct *src,
                              struct binder_transaction_data *tr) {
  // 1. Locate the target process via the handle (Not implemented)
  // 2. Allocate space in the target process's binder buffer
  // 3. Copy data from src to target (Zero-copy logic)

  // For now, we simulate a successful transaction
  return 0;
}

/**
 * android_binder_ioctl - Entry point for user-space communication.
 * This function handles the high-level protocol commands.
 */
long android_binder_ioctl(struct file *filp, unsigned int cmd,
                          unsigned long arg) {
  // Using current_task pointer instead of index for performance
  struct task_struct *curr = current_task;

  switch (cmd) {
  case BINDER_WRITE_READ: {
    struct binder_write_read bwr;
    mm_memcpy(&bwr, (void *)arg, sizeof(bwr)); // Use our fixed memcpy

    /* Handle incoming write commands (Transactions) */
    if (bwr.write_size > 0) {
      struct binder_transaction_data tr;
      // Read the transaction from the user buffer
      mm_memcpy(&tr, (void *)bwr.write_buffer, sizeof(tr));
      return binder_transaction(curr, &tr);
    }

    /* Handle read requests (Waiting for replies) */
    if (bwr.read_size > 0) {
      // In Linux, the thread would sleep here until data arrives
      return 0;
    }
    break;
  }

  case BINDER_SET_MAX_THREADS: {
    // Limits the number of threads for the binder pool
    return 0;
  }

  default:
    return -EINVAL; // Return standard error code
  }
  return 0;
}
