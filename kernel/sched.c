#include <kernel/sched.h>
#include <kernel/printk.h>

task_t* current_task = 0;

static task_t* ready_queue[MAX_TASKS];
static int task_count = 0;
static int current_idx = 0;

void sched_init(void) {
    task_count = 0;
    current_idx = 0;
    current_task = 0;
}

void sched_add_task(task_t* task) {
    if (task_count < MAX_TASKS) {
        ready_queue[task_count++] = task;
    } else {
        pr_err("SCHED", "Ready queue is full!", 2);
    }
}

void schedule(void) {
    if (task_count == 0) return;

    task_t* prev = current_task;

    current_idx = (current_idx + 1) % task_count;
    current_task = ready_queue[current_idx];

    if (prev != current_task && prev != 0) {
        // extern void arch_switch_context(task_t* old, task_t* new);
        // arch_switch_context(prev, current_task);
    }
}