#include <kernel/printk.h>
#include <kernel/process.h>
#include <kernel/sched.h>
#include <stddef.h>

/* Tell the compiler that the real pointer lives inside sched.c */
extern task_t *current_task;

/**
 * do_exit - Terminate the current process.
 */
void do_exit(int status) {
  task_t *curr = current_task;

  if (!curr) {
    return;
  }

  curr->state = ZOMBIE; /* Uses your task_state_t enum from task.h */
  curr->exit_code = status;

  boot_msg("TASK", "Process exited execution framework", 0);

  schedule();

  boot_msg("TASK", "Error, zombie process tried to return!", 2);
  
  while (1) {
#if defined(__riscv)
    asm volatile("wfi");
#elif defined(x86)
    asm volatile("cli; hlt");
#endif
  }
}