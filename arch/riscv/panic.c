#include <kernel/colors.h>
#include <kernel/printk.h>
#include <stdarg.h>
#include <stdint.h>

static void halt_system() {
  __asm__ volatile("csrc sstatus, %0" : : "r"(1 << 1));

  for (;;) {
    __asm__ volatile("wfi");
  }
}

void k_panic(const char *file, int line, const char *reason, ...) {
  __asm__ volatile("csrc sstatus, %0" : : "r"(1 << 1));

  printk("\nDUMP (reason: %s)\n{", reason);
  printk("  REASON:  ");

  va_list args;
  va_start(args, reason);
  vprintk(reason, args);
  va_end(args);

  printk("\n");

  if (file) {
    printk("src  :  %s\n", file);
    printk("line :  %d\n", line);
  }

  printk("\n  CPU REGISTERS (RISC-V):\n");

  uintptr_t ra, sp, gp, tp, a0, a1, t0, t1, s0;
  __asm__ volatile("mv %0, ra" : "=r"(ra));
  __asm__ volatile("mv %0, sp" : "=r"(sp));
  __asm__ volatile("mv %0, gp" : "=r"(gp));
  __asm__ volatile("mv %0, tp" : "=r"(tp));
  __asm__ volatile("mv %0, a0" : "=r"(a0));
  __asm__ volatile("mv %0, a1" : "=r"(a1));
  __asm__ volatile("mv %0, t0" : "=r"(t0));
  __asm__ volatile("mv %0, t1" : "=r"(t1));
  __asm__ volatile("mv %0, s0" : "=r"(s0));

  printk("  RA: 0x%p  SP: 0x%p  GP: 0x%p\n", ra, sp, gp);
  printk("  TP: 0x%p  A0: 0x%p  A1: 0x%p\n", tp, a0, a1);
  printk("  T0: 0x%p  T1: 0x%p  FP: 0x%p\n", t0, t1, s0);

  printk("\n  STACK BACKTRACE:\n");

  uintptr_t *fp = (uintptr_t *)s0;
  for (int i = 0; i < 6 && fp != 0; i++) {
    uintptr_t return_addr = fp[-1];
    if (return_addr == 0)
      break;

    printk("    [%d] at 0x%p\n", i, return_addr);

    uintptr_t *next_fp = (uintptr_t *)fp[-2];

    if (next_fp <= fp || (uintptr_t)next_fp > 0x90000000)
      break;
    fp = next_fp;
  }
  printk(" }");

  halt_system();
}