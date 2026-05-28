#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

#define TASK_RUNNING    0
#define TASK_ZOMBIE     1


struct task_struct* get_current_task();
void do_exit(int status);

#endif
