#ifndef _SYSCALL_H
#define _SYSCALL_H

/* POSIX Standard Syscalls */
#define SYS_READ   0
#define SYS_WRITE  1
#define SYS_OPEN   2

/* BlueOS Custom Syscalls */
#define SYS_PRINTK 4
#define SYS_CLEAR  5
#define SYS_EXIT   6

#endif /* _SYSCALL_H */