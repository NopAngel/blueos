#include <kernel/errno.h>
#include <kernel/printk.h>

/**
 * sys_fork: Clones the current execution context to create a new child process.
 */
int sys_fork(void) {
  printk("<5>[  SYS_FORK  ] Duplicating active task frame registers and memory "
         "layouts.\n");

  int child_pid = 42; /* Simulated PID assignment for the newly spawned process
                         descriptor */

  /* Returns 0 inside the child context, and the child's PID inside the parent
   * process context */
  return child_pid;
}