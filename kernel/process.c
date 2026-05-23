#include <kernel/process.h>
#include <kernel/sched.h>
#include <stddef.h>
#include <kernel/printk.h>

extern struct task_struct *current_task;

/**
 * get_current_task - Returns the task currently executing.
 */
struct task_struct* get_current_task() {
    return current_task;
}

/**
 * do_exit - Terminate the current process.
 * This is the internal implementation of the exit() system call.
 */
void do_exit(int status) {
    struct task_struct *curr = current_task;

    if (!curr) {
        /* If there's no current task, we are likely in an early kernel panic */
        return;
    }

    /* 1. Set the process state to ZOMBIE */
    /* A zombie process has finished execution but still has an entry in the task list */
    curr->state = TASK_ZOMBIE;
    curr->exit_code = status;

    pr_info("Process [%s] (PID:%d) exited with status %d\n",
           curr->name, curr->pid, status);

    /* 2. Notify the parent process (Optional: logic for wait() syscall) */
    // wake_up_process(curr->parent);

    /* 3. Schedule another task immediately */
    /* We never return from this call if the scheduler is working correctly */
    schedule();

    /* 4. Safety net: If schedule() returns, something is very wrong */
    pr_err("Process: Error, zombie process tried to return!\n");
    while(1) {
        /* Universal low-power wait */
#if defined(__riscv)
        asm volatile("wfi");
#elif defined(x86)
        asm volatile("hlt");
#endif
    }
}

