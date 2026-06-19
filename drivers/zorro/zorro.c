#include <kernel/printk.h>
#include <kernel/process.h>
#include <stdint.h>

#define ZORRO_LOG_SIZE 1024

static char zorro_log[ZORRO_LOG_SIZE];
static int log_ptr = 0;

void zorro_sniff(int pid, const char *action, uintptr_t addr) {

  int len = 0;
  while (action[len] && len < 16) {
    zorro_log[log_ptr % ZORRO_LOG_SIZE] = action[len];
    log_ptr++;
    len++;
  }

  zorro_log[log_ptr % ZORRO_LOG_SIZE] = (char)(pid & 0xFF);
  log_ptr++;
}
