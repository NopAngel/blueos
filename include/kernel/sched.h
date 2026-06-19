#ifndef SCHED_H
#define SCHED_H

#include <kernel/task.h>

extern task_t *current_task;

void schedule(void);
void sched_add_task(task_t *task);
void sched_init(void);

#endif