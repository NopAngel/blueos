#include <kernel/gpio.h>
#include <kernel/io.h>
#include <kernel/printk.h>
#include <stdint.h>

/* --- MMIO Base Addresses (Example for RISC-V SiFive/Virt) --- */
#if defined(__riscv)
#define GPIO_BASE_A 0x10012000
#define GPIO_BASE_B 0x10013000

/* Typical offsets for MMIO GPIO */
#define GPIO_VALUE 0x00
#define GPIO_INPUT_EN 0x04
#define GPIO_OUTPUT_EN 0x08
#define GPIO_PULLUP_EN 0x0C
#endif

/**
 * get_port_base - Internal helper to get the address/port base
 */
static uintptr_t get_port_base(uint8_t port) {
#if defined(__riscv)
  return (port == GPIO_PORT_A) ? GPIO_BASE_A : GPIO_BASE_B;
#else
  /* x86: Using dummy port addresses for logic demonstration */
  return (port == GPIO_PORT_A) ? 0x0300 : 0x0310;
#endif
}

/**
 * gpio_set_mode - Configures pin direction and pull-ups
 */
void gpio_set_mode(uint8_t port, uint8_t pin, uint8_t mode) {
  uintptr_t base = get_port_base(port);

#if defined(__riscv)
  volatile uint32_t *reg_out = (uint32_t *)(base + GPIO_OUTPUT_EN);
  volatile uint32_t *reg_in = (uint32_t *)(base + GPIO_INPUT_EN);
  volatile uint32_t *reg_pull = (uint32_t *)(base + GPIO_PULLUP_EN);

  if (mode == GPIO_OUTPUT) {
    *reg_out |= (1 << pin);
    *reg_in &= ~(1 << pin);
  } else if (mode == GPIO_INPUT || mode == GPIO_INPUT_PULLUP) {
    *reg_out &= ~(1 << pin);
    *reg_in |= (1 << pin);
    if (mode == GPIO_INPUT_PULLUP) {
      *reg_pull |= (1 << pin);
    }
  }
#elif defined(__x86__) || defined(__x86_64__)
  /* x86: Hardware dependent (usually via SuperIO or Chipset) */
  // Placeholder: printk("GPIO: Setting x86 port 0x%x pin %d to mode %d\n",
  // base, pin, mode);
#endif
}

/**
 * gpio_write_pin - Writes HIGH or LOW to a pin
 */
void gpio_write_pin(uint8_t port, uint8_t pin, uint8_t level) {
  uintptr_t base = get_port_base(port);

#if defined(__riscv)
  volatile uint32_t *reg_val = (uint32_t *)(base + GPIO_VALUE);
  if (level == GPIO_HIGH) {
    *reg_val |= (1 << pin);
  } else {
    *reg_val &= ~(1 << pin);
  }
#elif defined(__x86__) || defined(__x86_64__)
  /* x86: Standard I/O port write if mapped */
  uint8_t current = inb(base);
  if (level == GPIO_HIGH) {
    outb(base, current | (1 << pin));
  } else {
    outb(base, current & ~(1 << pin));
  }
#endif
}

/**
 * gpio_read_pin - Reads the state of a pin
 */
uint8_t gpio_read_pin(uint8_t port, uint8_t pin) {
  uintptr_t base = get_port_base(port);

#if defined(__riscv)
  volatile uint32_t *reg_val = (uint32_t *)(base + GPIO_VALUE);
  return (*reg_val & (1 << pin)) ? GPIO_HIGH : GPIO_LOW;
#elif defined(__x86__) || defined(__x86_64__)
  return (inb(base) & (1 << pin)) ? GPIO_HIGH : GPIO_LOW;
#else
  return GPIO_LOW;
#endif
}