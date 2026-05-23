#ifndef XEN_H
#define XEN_H

#include <stdint.h>

#define __HYPERVISOR_console_io  18
#define __HYPERVISOR_sched_op    29
#define __HYPERVISOR_event_channel_op 32

/* Multi-arch Hypercall */
static inline long xen_hypercall(int nr, long a1, long a2, long a3) {
    long __res;
#if defined(__x86__)
    asm volatile (
        "int $0x82"
        : "=a" (__res)
        : "0" (nr), "b" (a1), "c" (a2), "d" (a3)
        : "memory"
    );
#elif defined(__riscv)
    register long a0 asm("a0") = a1;
    register long a1_reg asm("a1") = a2;
    register long a2_reg asm("a2") = a3;
    register long t0 asm("t0") = nr;
    asm volatile (
        "ecall"
        : "=r" (a0)
        : "r" (t0), "r" (a0), "r" (a1_reg), "r" (a2_reg)
        : "memory"
    );
    __res = a0;
#endif
    return __res;
}

#endif