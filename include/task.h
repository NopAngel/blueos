
#include <include/types.h>

typedef struct {
    uint32_t esp;          // Stack Pointer actual
    uint32_t kstack_base; 
    int id;
    int state;             
    char name[32];
} task_t;

#define MAX_TASKS 4
#define STACK_SIZE 4096