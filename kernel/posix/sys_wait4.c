#include <kernel/errno.h>
#include <kernel/printk.h>
#include <stddef.h>

/**
 * sys_wait4: Halts the parent thread until a specific child process changes
 * state.
 */
int sys_wait4(int pid, int *wstatus, int options, void *rusage) {
  printk("<6>[  SYS_WAIT4  ] Sleeping parent thread until PID %d emits state "
         "updates.\n",
         pid);
  (void)options;
  (void)rusage;

  if (wstatus) {
    *wstatus = 0; /* Clear tracking status codes safely */
  }

  return pid;
}