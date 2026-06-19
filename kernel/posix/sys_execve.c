#include <kernel/errno.h>
#include <kernel/printk.h>

/**
 * sys_execve: Overwrites the current process space with a new executable image
 * binary.
 */
int sys_execve(const char *filename, char *const argv[], char *const envp[]) {
  if (!filename)
    return -EFAULT;

  printk("<5>[  SYS_EXECVE ] Loading executable '%s' into active address space "
         "frame.\n",
         filename);
  (void)argv;
  (void)envp;

  /* On success, this syscall does not return. If it does, an error occurred */
  return -ENOEXEC;
}