#include <kernel/errno.h>
#include <kernel/printk.h>
#include <stdint.h>

#define MODULE_NAME "COMPAT_SIG"

/* 32-bit Signal Set layout architecture */
typedef struct {
  uint32_t sig[2]; /* 2 * 32 bits = 64 signals tracking capacity mask */
} compat_sigset_t;

/* Native 64-bit Kernel Signal Set layout mapping */
typedef uint64_t native_sigset_t;

/**
 * compat_to_native_sigset: Translates a split 32-bit signal mask into a single
 * 64-bit word.
 */
static void compat_to_native_sigset(const compat_sigset_t *compat,
                                    native_sigset_t *native) {
  *native = ((native_sigset_t)compat->sig[1] << 32) | compat->sig[0];
}

/**
 * compat_sys_rt_sigprocmask: Modifies the calling thread's blocked signal mask
 * from a 32-bit task.
 */
long compat_sys_rt_sigprocmask(int how, const compat_sigset_t *set,
                               compat_sigset_t *oset, size_t sigsetsize) {
  printk("<5>[  %s  ] Translating rt_sigprocmask attributes from legacy task "
         "context.\n",
         MODULE_NAME);
  (void)sigsetsize;

  native_sigset_t native_set = 0;
  native_sigset_t native_oset = 0;

  if (set) {
    compat_to_native_sigset(set, &native_set);
    printk("<7>[  %s  ] Unpacked 32-bit compound signal mask value: 0x%llX\n",
           MODULE_NAME, native_set);
  }

  /* Core signal routing logic updates the thread structure here */
  // extern int do_sigprocmask(int how, native_sigset_t *set, native_sigset_t
  // *oset); int ret = do_sigprocmask(how, set ? &native_set : NULL,
  // &native_oset);

  if (oset) {
    oset->sig[0] = (uint32_t)(native_oset & 0xFFFFFFFF);
    oset->sig[1] = (uint32_t)(native_oset >> 32);
  }

  return 0;
}