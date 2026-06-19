#include <kernel/errno.h>
#include <kernel/printk.h>
#include <stdint.h>

#define MODULE_NAME "RWSEM"

typedef struct {
  volatile int32_t
      activity_count; /* 0 = Idle, >0 = Total Readers, -1 = Writer Locked */
} rwsemaphore_t;

/**
 * rwsem_init: Provisions standard base state contexts.
 */
void rwsem_init(rwsemaphore_t *rwsem) { rwsem->activity_count = 0; }

/**
 * rwsem_down_read: Locks access path for a concurrent reader node.
 */
void rwsem_down_read(rwsemaphore_t *rwsem) {
  while (1) {
    int32_t current = rwsem->activity_count;
    if (current >= 0) {
      if (__sync_bool_compare_and_swap(&rwsem->activity_count, current,
                                       current + 1)) {
        printk("<7>[  %s  ] Secured read handle path. Active readers: %d\n",
               MODULE_NAME, current + 1);
        break;
      }
    } else {
      /* A writer has exclusive hardware locking domains active; spin/pause wait
       */
      asm volatile("pause");
    }
  }
}

/**
 * rwsem_up_read: Signals termination of reader node lookups.
 */
void rwsem_up_read(rwsemaphore_t *rwsem) {
  __sync_fetch_and_sub(&rwsem->activity_count, 1);
}

/**
 * rwsem_down_write: Claims absolute exclusive blocking control over both
 * readers and writers.
 */
void rwsem_down_write(rwsemaphore_t *rwsem) {
  while (1) {
    if (__sync_bool_compare_and_swap(&rwsem->activity_count, 0, -1)) {
      printk("<6>[  %s  ] Exclusive write lock channel secured. Matrix fully "
             "isolated.\n",
             MODULE_NAME);
      break;
    }
    asm volatile("pause");
  }
}

/**
 * rwsem_up_write: Restores the state structure back to zero to clear execution
 * tracks.
 */
void rwsem_up_write(rwsemaphore_t *rwsem) {
  __sync_bool_compare_and_swap(&rwsem->activity_count, -1, 0);
  printk("<7>[  %s  ] Write channel closed. Base structures open for public "
         "queue requests.\n",
         MODULE_NAME);
}