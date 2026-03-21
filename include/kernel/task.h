
#include <kernel/types.h>
#include <stdint.h> 

typedef struct {
    int id;
    int state;
    char name[32];
    uint64_t rsp;   
} task_t;

struct fpu_context {
    uint64_t fregs[32]; 
    uint32_t fcsr;      
};

#define MAX_TASKS 4
#define STACK_SIZE 4096