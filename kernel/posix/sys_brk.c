#include <kernel/printk.h>
#include <stddef.h>

static uintptr_t g_current_program_break =
    0x4000000; /* Simulated heap start boundary */

/**
 * sys_brk: Controls the absolute terminal boundary limit of the data segment
 * (Heap allocation resizing).
 */
uintptr_t sys_brk(uintptr_t addr) {
  printk("<7>[  SYS_BRK    ] Requesting memory segment breakpoint resize to "
         "address: %p\n",
         (void *)addr);

  if (addr == 0) {
    return g_current_program_break;
  }

  g_current_program_break = addr;
  return g_current_program_break;
}