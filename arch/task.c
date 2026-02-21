#include <include/task.h>
#include <include/lib/string.h>

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
        task_list[i].esp = 0;
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

  
    uint32_t* stack = (uint32_t*)(&task_stacks[id][STACK_SIZE]);


    *(--stack) = (uint32_t)entry_point; 

    *(--stack) = 0; // EBP
    *(--stack) = 0; // EDI
    *(--stack) = 0; // ESI 
    *(--stack) = 0; // EBX 

  
    t->esp = (uint32_t)stack;
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

    if (next == -1 || next == last) {
        return;
    }

    current_task_id = next;

    extern void switch_to_task(uint32_t* old_esp_ptr, uint32_t new_esp);
    
    switch_to_task(&(task_list[last].esp), task_list[next].esp);
}