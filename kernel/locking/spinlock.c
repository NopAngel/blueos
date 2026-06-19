#include <kernel/colors.h>
#include <kernel/printk.h>
#include <stdint.h>

#define MODULE_NAME "SPINLOCK"

typedef struct {
  volatile uint32_t lock_state; /* 0 = Unlocked, 1 = Locked */
} spinlock_t;

/**
 * spinlock_init: Initializes the spinlock handle descriptor to an unlocked
 * state.
 */
void spinlock_init(spinlock_t *lock) { lock->lock_state = 0; }

/**
 * spinlock_lock: Spins actively in a tight loop utilizing atomic hardware
 * operations.
 */
void spinlock_lock(spinlock_t *lock) {
  uint32_t acquired = 1;

  /* Atomic Test-And-Set loop block using inline x86 assembly */
  while (acquired) {
    asm volatile("lock xchg %0, %1"
                 : "+r"(acquired), "+m"(lock->lock_state)
                 :
                 : "memory");

    if (acquired) {
      /* PAUSE instruction hints the CPU that we are in a spin-loop to optimize
       * power */
      asm volatile("pause");
    }
  }
}

/**
 * spinlock_unlock: Atomically releases the spinlock vector.
 */
void spinlock_unlock(spinlock_t *lock) {
  asm volatile("lock xchg %0, %1" : : "r"(0), "m"(lock->lock_state) : "memory");
}