#include <kernel/printk.h>
#include <stdint.h>

#define MODULE_NAME "SEMAPHORE"

typedef struct {
  volatile int count;
  uint32_t total_waiters;
} semaphore_t;

/**
 * sem_init: Initializes the semaphore capacity boundary counter tokens.
 */
void sem_init(semaphore_t *sem, int initial_count) {
  sem->count = initial_count;
  sem->total_waiters = 0;
}

/**
 * sem_down: Decrements count or stalls execution if the resource boundary
 * limits are breached.
 */
void sem_down(semaphore_t *sem) {
  while (1) {
    /* Atomic counter inspection safe loop step */
    int current = sem->count;
    if (current > 0) {
      if (__sync_bool_compare_and_swap(&sem->count, current, current - 1)) {
        break; /* Successfully allocated token spot */
      }
    } else {
      /* No tokens available, wait on the execution line */
      __sync_fetch_and_add(&sem->total_waiters, 1);
      printk("<7>[  %s  ] Limit reached. Blocking task on synchronization "
             "queue.\n",
             MODULE_NAME);

      /* Execution delay fallback step or scheduler yield loop */
      for (volatile int i = 0; i < 10000; i++)
        ;

      __sync_fetch_and_sub(&sem->total_waiters, 1);
    }
  }
}

/**
 * sem_up: Increments tracking tokens and signals pending line consumers.
 */
void sem_up(semaphore_t *sem) {
  __sync_fetch_and_add(&sem->count, 1);
  if (sem->total_waiters > 0) {
    printk("<6>[  %s  ] Signaling and wake up of blocked queue thread "
           "triggered.\n",
           MODULE_NAME);
  }
}