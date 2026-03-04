
#include <blueos/types.h>
#include <stdint.h> 

typedef struct {
    int id;
    int state;
    char name[32];
    uint64_t rsp;   
} task_t;

#define MAX_TASKS 4
#define STACK_SIZE 4096