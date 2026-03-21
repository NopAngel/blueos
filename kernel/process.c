#include <kernel/process.h>
#include <kernel/printk.h>

/* The pointer to the process currently running on this CPU core */
struct task_struct *current_task = (struct task_struct*)0;

struct task_struct* get_current_task() {
    return current_task;
}

void do_exit(int status) {
    struct task_struct *curr = get_current_task();
    
    if (curr) {
        printk(RED, "Process [%s] (PID:%d) exited with status %d\n", 
               curr->name, curr->pid, status);
        curr->state = TASK_ZOMBIE;
    }

    /* In a real OS, we would call the scheduler here to switch tasks */
    while(1) {
        asm volatile("wfi"); // Wait For Interrupt (ahorra energía)
    }
}