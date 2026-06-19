#include <kernel/printk.h>

void zorro_report() {
  printk("\n--- [ Zorro Stealth Report ] ---\n");
  for (int i = 0; i < log_ptr && i < ZORRO_LOG_SIZE; i++) {
    if (zorro_log[i] >= 32 && zorro_log[i] <= 126) {
      printk("%c", zorro_log[i]);
    }
  }
  printk("\n--- [ End of Trail ] ---\n");
}