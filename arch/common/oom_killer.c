#include <kernel/printk.h>
#include <mm/memory.h>
#include <stddef.h>
#include <stdint.h>

#define MODULE_NAME "OOM_KILLER"

/* Simulated process control block layout for BlueOS context tracking */
typedef struct {
  int pid;
  char name[16];
  uint32_t pages_used;
  int protected; /* Kernel tasks or critical services cannot be killed */
} oom_proc_t;

/* Mock internal process table */
static oom_proc_t proc_table[] = {
    {1, "init.sys", 124, 1},
    {2, "qsh.c", 45, 0},
    {3, "bad_leak_app", 8192, 0}, /* Clear target for execution */
    {4, "vfs_service", 256, 1}};
static const int proc_count = 4;

/**
 * oom_select_bad_process: Iterates over tasks to select the highest consumption
 * candidate.
 */
static oom_proc_t *oom_select_bad_process(void) {
  uint32_t max_pages = 0;
  oom_proc_t *candidate = NULL;

  for (int i = 0; i < proc_count; i++) {
    if (proc_table[i].protected)
      continue;

    if (proc_table[i].pages_used > max_pages) {
      max_pages = proc_table[i].pages_used;
      candidate = &proc_table[i];
    }
  }
  return candidate;
}

/**
 * mm_out_of_memory_rescue: Emergency invocation triggered upon physical memory
 * starvation.
 */
void mm_out_of_memory_rescue(void) {
  printk("<1>[  %s  ] Critical resource depletion detected! Invoking rescue "
         "handler.\n",
         MODULE_NAME);

  oom_proc_t *victim = oom_select_bad_process();

  if (!victim) {
    /* No user tasks can be sacrificed, system must crash through your native
     * k_panic */
    extern void k_panic(int code, const char *reason);
    k_panic(0x0001, "OOM Fatal Error: Absolute memory starvation. Zero "
                    "eligible processes to kill.");
    return;
  }

  printk("<0>[  %s  ] Killing process %d (%s) to reclaim %u pages.\n",
         MODULE_NAME, victim->pid, victim->name, victim->pages_used);

  /* Simulated page reclamation routine: clearing mock usage counters */
  victim->pages_used = 0;

  printk("<5>[  %s  ] Memory pressure relieved. Control returned to execution "
         "context.\n",
         MODULE_NAME);
}