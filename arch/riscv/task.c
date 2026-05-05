#include <kernel/task.h>
#include <kernel/sched.h>
#include <kernel/printk.h>
#include <lib/string.h>

#define MAX_TASKS 4
#define STACK_SIZE 4096

static struct task_struct task_pool[MAX_TASKS];
static int allocated_tasks = 0;
int current_task_id = 0;

void task_init() {
    memset(&task_pool[0], 0, sizeof(struct task_struct));
    strcpy(task_pool[0].name, "Kernel");
    task_pool[0].pid = 0;
    task_pool[0].state = TASK_READY;

    allocated_tasks = 1;
    current_task_id = 0;

    printk(WHITE, "[INFO ] Task system initialized. Kernel is PID 0\n");
}

struct task_struct *create_task(void (*entry)(void), const char* name) {
    if (allocated_tasks >= MAX_TASKS) {
        printk(RED, "Task: Limit reached!\n");
        return NULL;
    }

    struct task_struct *t = &task_pool[allocated_tasks++];
    memset(t, 0, sizeof(struct task_struct));
    strcpy(t->name, name);

    uint32_t *stack_ptr = (uint32_t *)(t->stack + STACK_SIZE);
    stack_ptr -= 16;

    stack_ptr[0] = (uint32_t)entry;

    for (int i = 1; i < 16; i++) {
        stack_ptr[i] = 0;
    }

    t->esp   = (uint32_t)stack_ptr;
    t->pid   = allocated_tasks - 1;
    t->state = TASK_READY;

    printk(WHITE, "[INFO ] Task %d (%s) created at %p\n", t->pid, name, entry);
    return t;
}

void yield() {
    if (allocated_tasks <= 1) return;

    int last = current_task_id;
    int next = (last + 1) % allocated_tasks;

    current_task_id = next;

    extern void switch_to_task(uint32_t* old_sp_ptr, uint32_t new_sp);

    switch_to_task(&(task_pool[last].esp), task_pool[next].esp);
}
