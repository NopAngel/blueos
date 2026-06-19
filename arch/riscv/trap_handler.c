#include <arch/riscv/plic.h>
#include <arch/riscv/timer.h>
#include <kernel/colors.h>
#include <kernel/printk.h>
#include <kernel/sched.h>

struct registers {
  uint32_t zero, ra, sp, gp, tp, t0, t1, t2;
  uint32_t s0, s1, a0, a1, a2, a3, a4, a5;
  uint32_t a6, a7, s2, s3, s4, s5, s6, s7;
  uint32_t s8, s9, s10, s11, t3, t4, t5, t6;
  uint32_t scause, stval, sepc, sstatus;
};

void irq_handler(struct registers *r) {
  uint32_t irq = plic_claim();

  if (irq == 10) {
    printk("\033[36mUART Interrupt: Key pressed (maybe)!\033[0m\n");
  } else {
    printk("IRQ External: %d\n", irq);
  }

  plic_complete(irq);
}

void exception_handler(struct registers *r) {
  printk("\n--- KERNEL PANIC: Exception ---\n");
  printk("scause: 0x%x\n", r->scause);
  printk("sepc  : 0x%x\n", r->sepc);
  printk("stval : 0x%x\n", r->stval);

  printk("ra: 0x%x, sp: 0x%x, a0: 0x%x\n", r->ra, r->sp, r->a0);

  for (;;)
    ;
}

void main_trap_handler(struct registers *r) {
  if ((r->scause >> 31) & 1) {
    irq_handler(r);
  } else {
    exception_handler(r);
  }
}

void handle_trap(uint32_t sp) {
  uint32_t mcause;
  asm volatile("csrr %0, mcause" : "=r"(mcause));

  if (mcause == 0x80000007) {
    timer_set_next(100000);
    schedule();
  }
}
