#include <kernel/task.h>
#include <lib/string.h>

int process_count = 1;
#define MAX_TASKS 4
#define STACK_SIZE 4096

task_t task_list[MAX_TASKS];
int current_task_id = 0;


uint8_t task_stacks[MAX_TASKS][STACK_SIZE] __attribute__((aligned(16)));

void task_init() {
    for (int i = 0; i < MAX_TASKS; i++) {
        task_list[i].id = i;
        task_list[i].state = 0;
        memset(task_list[i].name, 0, 32);
        task_list[i].rsp = 0; 
    }

    task_list[0].state = 1;
    strcpy(task_list[0].name, "Kernel");
    current_task_id = 0;
}

void create_task(void (*entry_point)(), char* name) {
    int id = -1;

    for (int i = 1; i < MAX_TASKS; i++) {
        if (task_list[i].state == 0) {
            id = i;
            break;
        }
    }

    if (id == -1) return; 

    task_t* t = &task_list[id];
    t->state = 1;
    strcpy(t->name, name);

    uint64_t* stack = (uint64_t*)(&task_stacks[id][STACK_SIZE]);

    
    *(--stack) = (uint64_t)entry_point; 

    *(--stack) = 0; // RBP
    *(--stack) = 0; // RBX
    *(--stack) = 0; // R12
    *(--stack) = 0; // R13
    *(--stack) = 0; // R14
    *(--stack) = 0; // R15

    t->rsp = (uint64_t)stack;
}

void yield() {
    int last = current_task_id;
    int next = -1;

    for (int i = 1; i <= MAX_TASKS; i++) {
        int index = (last + i) % MAX_TASKS;
        if (task_list[index].state == 1) {
            next = index;
            break;
        }
    }

    if (next == -1 || next == last) return;

    current_task_id = next;

    extern void switch_to_task(uint64_t* old_rsp_ptr, uint64_t new_rsp);
    
    switch_to_task(&(task_list[last].rsp), task_list[next].rsp);
}