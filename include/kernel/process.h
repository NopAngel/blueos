#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include <stdbool.h>
#include <kernel/task.h>

#define TASK_RUNNING 1  /* Asegúrate de que coincida con tus enums si los usas */
#define TASK_ZOMBIE  3

/* Pledge Categories (Bitmask flags) */
#define PLEDGE_STDIO    (1 << 0)
#define PLEDGE_RPATH    (1 << 1)
#define PLEDGE_WPATH    (1 << 2)
#define PLEDGE_CPATH    (1 << 3)
#define PLEDGE_DRV      (1 << 4)

/* Prototipos globales */
struct task_struct *get_current_task(void);
void do_exit(int status);

#endif