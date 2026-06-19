#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <stdint.h>

typedef struct {
  volatile uint32_t locked;
} spinlock_t;

static inline void spin_lock(spinlock_t *lock) {
  while (__sync_lock_test_and_set(&lock->locked, 1)) {
    while (lock->locked) {
      __asm__ volatile("pause");
    }
  }
}

static inline void spin_unlock(spinlock_t *lock) {
  __sync_lock_release(&lock->locked);
}

#define SPINLOCK_INIT {0}

#endif