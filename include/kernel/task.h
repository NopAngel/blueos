#ifndef TASK_H
#define TASK_H

#include <uapi/blueos/signal.h>
#include <stdint.h>

#define STACK_SIZE 8192

struct fpu_context {
    uint8_t data[512];
} __attribute__((aligned(16)));
typedef enum {
    READY,
    RUNNING,
    SLEEPING,
    ZOMBIE
} task_state_t;


typedef struct task_struct {
    uint32_t esp;            
    uint32_t pid; 
    uint32_t state;
    int exit_code;            
    char name[32];         
    struct fpu_context fpu;  
    uint32_t stack_base;  
    uint32_t pending_signals;    
    sig_handler_t signal_handlers[32];
    
    struct trap_frame *tf;  
    int fds[16]; // Tabla de descriptores de archivo (File Descriptors)
} task_t;


#define MAX_TASKS 32

int kthread_create(void (*fn)(void), const char* name);

#endif