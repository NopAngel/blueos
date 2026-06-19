#ifndef _SYSCALL_USER_H
#define _SYSCALL_USER_H

#include <kernel/syscall.h>
#include <stdint.h>

/**
 * Wrapper genérico para syscalls con 2 argumentos
 */
static inline void _syscall_2(int num, uint32_t arg1, uint32_t arg2) {
  asm volatile("int $0x80" : : "a"(num), "b"(arg1), "c"(arg2) : "memory");
}

/**
 * Wrapper genérico para syscalls con 0 argumentos
 */
static inline void _syscall_0(int num) {
  asm volatile("int $0x80" : : "a"(num) : "memory");
}

static inline void sys_print(uint8_t color, const char *msg) {
  _syscall_2(SYS_PRINTK, (uint32_t)msg, (uint32_t)color);
}

static inline void sys_exit(int code) {
  _syscall_2(SYS_EXIT, (uint32_t)code, 0);
}

#endif