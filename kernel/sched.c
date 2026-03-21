#include <kernel/sched.h>
#include <kernel/printk.h>

struct task_struct *tasks[MAX_TASKS];
int current_task_index = 0;

void schedule(void) {
    struct task_struct *prev = tasks[current_task_index];

    current_task_index = (current_task_index + 1) % MAX_TASKS;
    struct task_struct *next = tasks[current_task_index];

    if (next->state == TASK_READY) {
        next->state = TASK_RUNNING;
        switch_to(prev, next); 
    }
}

int scheduler_init(void) {

    pr_info("Sched: Round-Robin scheduler initialized (100Hz)\n");
    return 0;
}
core_initcall(scheduler_init);