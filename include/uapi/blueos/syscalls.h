#ifndef _UAPI_BLUEOS_SYSCALLS_H
#define _UAPI_BLUEOS_SYSCALLS_H

#define SYS_read     0
#define SYS_write    1
#define SYS_open     2
#define SYS_close    3
#define SYS_exit     60
#define SYS_getpid   39
#define SYS_malloc   45  /* brk/sbrk */

typedef struct {
    int pid;
    int state;
    char name[32];
} proc_info_t;

#endif