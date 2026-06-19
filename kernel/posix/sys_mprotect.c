#include <kernel/printk.h>
#include <stddef.h>

/* Binds into your native memory protection subsystem mapped inside
 * arch/common/mprotect.c */
extern int mm_mprotect_set_flags(uintptr_t address, size_t length,
                                 int prot_flags);

/**
 * sys_mprotect: Syscall wrapper interface to set permissions on active process
 * memory frames.
 */
int sys_mprotect(void *addr, size_t len, int prot) {
  printk("<5>[  SYS_MPROTECT ] Syscall received: validating range %p to modify "
         "attributes.\n",
         addr);

  return mm_mprotect_set_flags((uintptr_t)addr, len, prot);
}