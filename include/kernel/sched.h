#ifndef SCHED_H
#define SCHED_H

#include <stdint.h>

#define MAX_TASKS 16
#define STACK_SIZE 4096

typedef enum {
    TASK_RUNNING,
    TASK_READY,
    TASK_SLEEPING,
    TASK_ZOMBIE
} task_state_t;

struct task_struct {
    uint32_t esp;               
    uint32_t pid;             
    task_state_t state;  
    char name[32];      
    uint8_t stack[STACK_SIZE]; 
};

extern struct task_struct *tasks[MAX_TASKS];

#endif