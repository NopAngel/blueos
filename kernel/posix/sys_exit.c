#include <kernel/printk.h>

/**
 * sys_exit: Shuts down the current execution task and cleans up its tracking
 * descriptors.
 */
void sys_exit(int status) {
  printk("<0>[  SYS_EXIT   ] Terminating process token context with status "
         "code: %d\n",
         status);

  /* Free task context loop or yield to task scheduler */
  while (1)
    ;
}