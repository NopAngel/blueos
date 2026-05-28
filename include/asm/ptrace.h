#ifndef PTRACE_H
#define PTRACE_H

#include <stdint.h>

/* * This struct must match the order in which you push 
 * registers onto the stack in your assembly trap handler.
 */
struct pt_regs {
    unsigned long sepc;   /* Supervisor Exception Program Counter */
    unsigned long ra;     /* Return Address */
    unsigned long sp;     /* Stack Pointer */
    unsigned long gp;     /* Global Pointer */
    unsigned long tp;     /* Thread Pointer */
    unsigned long t0, t1, t2;
    unsigned long s0, s1;
    unsigned long a0, a1, a2, a3, a4, a5, a6, a7;
    unsigned long s2, s3, s4, s5, s6, s7, s8, s9, s10, s11;
    unsigned long t3, t4, t5, t6;
    unsigned long sstatus;
    unsigned long sbadaddr; /* Alias for stval */
    unsigned long scause;
};

#endif