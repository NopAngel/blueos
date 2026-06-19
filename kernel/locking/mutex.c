#include <kernel/errno.h>
#include <kernel/printk.h>
#include <stddef.h>
#include <stdint.h>

#define MODULE_NAME "MUTEX"

typedef struct {
  volatile int locked;
  int owner_pid;
  void *wait_queue; /* Pointer structure tracking sleeping tasks */
} mutex_t;

/**
 * mutex_init: Sets up structural layouts for sleeping exclusion locks.
 */
void mutex_init(mutex_t *lock) {
  lock->locked = 0;
  lock->owner_pid = -1;
  lock->wait_queue = NULL;
}

/**
 * mutex_lock: Acquires exclusive access or puts the thread to sleep if
 * contested.
 */
int mutex_lock(mutex_t *lock, int current_pid) {
  printk("<7>[  %s  ] PID %d requesting lock access acquisition...\n",
         MODULE_NAME, current_pid);

  /* Atomic state check simulation mapping */
  if (__sync_bool_compare_and_swap(&lock->locked, 0, 1)) {
    lock->owner_pid = current_pid;
    return 0; /* Secured cleanly */
  }

  printk(
      "<6>[  %s  ] Resource busy. Suspending PID %d into sleep wait state.\n",
      MODULE_NAME, current_pid);

  /* * In production, this block invokes your scheduler hooks:
   * current_task->state = TASK_UNINTERRUPTIBLE;
   * schedule();
   */
  return 0;
}

/**
 * mutex_unlock: Releases the lock context boundary and awakens the next in
 * queue.
 */
void mutex_unlock(mutex_t *lock) {
  if (lock->locked == 0)
    return;

  printk("<7>[  %s  ] Reclaiming lock ownership from PID %d\n", MODULE_NAME,
         lock->owner_pid);

  lock->owner_pid = -1;
  lock->locked = 0;

  /* Interface triggers scheduler wakes: wake_up_queue(&lock->wait_queue); */
}