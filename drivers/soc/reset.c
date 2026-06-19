#include <arch/x86/io.h>
#include <stdint.h>

void sys_reset__soc() {
  printk("[RESET] Initiating reboot sequence...\n");
  printk("Attempting 8042 Pulse...\n");
  uint8_t temp;

  do {
    temp = inb(0x64);
  } while (temp & 0x02);

  outb(0x64, 0xFE);

  printk("Attempting PCI System Reset (0xCF9)...\n");
  outb(0xCF9, 0x06); // 0x04 | 0x02

  printk("Forcing Triple Fault...\n");

  struct {
    uint16_t limit;
    uint32_t base;
  } __attribute__((packed)) idt_zero = {0, 0};

  asm volatile("lidt %0; int3" : : "m"(idt_zero));

  while (1)
    asm volatile("hlt");
}