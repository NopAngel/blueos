#ifndef TASK_H
#define TASK_H

#include <stdint.h>
#include <stdbool.h>
#include <uapi/blueos/signal.h>

#define STACK_SIZE 8192
#define MAX_UNVEIL_RULES 16

struct fpu_context {
  uint8_t data[512];
} __attribute__((aligned(16)));

typedef enum { READY, RUNNING, SLEEPING, ZOMBIE } task_state_t;

struct unveil_rule {
  char path[128];
  uint32_t permissions;
  bool active;
};

typedef struct task_struct {
  uint32_t esp;
  uint32_t pid;
  uint32_t state;
  int exit_code;
  char name[32];
  struct fpu_context fpu;
  uint32_t stack_base;
  uint32_t pending_signals;
  sig_handler_t signal_handlers[32];

  struct trap_frame *tf;
  int fds[16]; 

  bool pledge_active;
  uint32_t pledge_mask;
  struct unveil_rule unveil_rules[MAX_UNVEIL_RULES];
  uint32_t unveil_count;
} task_t;

#define MAX_TASKS 32

int kthread_create(void (*fn)(void), const char *name);

#endif