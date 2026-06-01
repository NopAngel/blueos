#include <kernel/malloc.h>
#include <kernel/printk.h>
#include <kernel/sched.h>
#include <kernel/task.h>
#include <lib/string.h>
#include <mm/memory.h>

task_t *running_task;

static uint32_t next_pid = 1;
extern void *arch_prepare_stack(void *stack_top, void (*fn)(void));

/**
 * Inicializa los descriptores estándar (POSIX)
 */
static void init_task_fds(task_t *t) {
  for (int i = 0; i < 16; i++)
    t->fds[i] = -1;
  t->fds[0] = 0; // stdin (consola)
  t->fds[1] = 1; // stdout (consola)
  t->fds[2] = 2; // stderr (consola)
}

void task_init() {
  sched_init();

  task_t *kernel_task = (task_t *)kmalloc(sizeof(task_t));
  memset(kernel_task, 0, sizeof(task_t));

  kernel_task->pid = 0;
  strcpy(kernel_task->name, "kernel_main");

  init_task_fds(kernel_task);
  current_task = kernel_task;
  sched_add_task(kernel_task);

  boot_msg("TASK", "Tasking system initialized with kernel_task", 0);
}

task_t *create_task(void (*entry)(), char *name) {
  task_t *t = (task_t *)kmalloc(sizeof(task_t));
  memset(t, 0, sizeof(task_t));

  uint32_t *stack = (uint32_t *)kmalloc(4096);
  uint32_t *esp = (uint32_t *)((uint32_t)stack + 4096);

  *(--esp) = (uint32_t)entry; /* (EIP) */
  *(--esp) = 0;               /* EBP */
  *(--esp) = 0;               /* EBX */
  *(--esp) = 0;               /* ESI */
  *(--esp) = 0;               /* EDI */

  t->esp = (uint32_t)esp;
  t->pid = 1;
  t->stack_base = (uint32_t)stack;
  init_task_fds(t);
  strncpy(t->name, name, 31);

  sched_add_task(t);
  return t;
}

void yield() { schedule(); }

task_t *get_current_task() { return running_task; }

int kthread_create(void (*fn)(void), const char *name) {
  /* Allocate memory for the task structure and its stack */
  task_t *new_task = (task_t *)kmalloc(sizeof(task_t));
  void *stack = kmalloc(STACK_SIZE);

  if (!new_task || !stack) {
    return -1;
  }

  /* Clean the structure */
  memset(new_task, 0, sizeof(task_t));

  /* Initialize basic fields */
  new_task->pid = next_pid++;
  new_task->state = READY;
  init_task_fds(new_task);
  new_task->stack_base = (uint32_t)stack;

  /* Copy name safely */
  if (name) {
    strncpy(new_task->name, name, 31);
  }

  /**
   * Prepare the stack for the architecture.
   * We pass the top of the stack (stack + STACK_SIZE).
   * The result is cast to uint32_t to match your struct's 'esp' field.
   */
  new_task->esp =
      (uint32_t)arch_prepare_stack((void *)((uintptr_t)stack + STACK_SIZE), fn);

  /* Register the task in the scheduler */
  sched_add_task(new_task);

  return new_task->pid;
}