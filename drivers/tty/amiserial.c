#include <kernel/colors.h>
#include <kernel/errno.h>
#include <kernel/printk.h>
#include <stdint.h>

#define MODULE_NAME "AMIGA_SERIAL"

/* Custom Amiga Hardware Register Offsets (Paula Chip) */
#define CUSTOM_BASE 0xDFF000
#define AMIGA_SERDATR                                                          \
  ((volatile uint16_t *)(CUSTOM_BASE + 0x018)) /* Serial data read (periphs)   \
                                                */
#define AMIGA_SERDAT                                                           \
  ((volatile uint16_t *)(CUSTOM_BASE + 0x030)) /* Serial data write */
#define AMIGA_SERPER                                                           \
  ((volatile uint16_t *)(CUSTOM_BASE + 0x032)) /* Serial period (baud rate) */

/**
 * amiserial_putc: Drops a single character raw byte into the Amiga hardware
 * transmitter buffer.
 */
void amiserial_putc(char c) {
  /* In real Amiga hardware, we wait for bit 15 (TBE - Transmit Buffer Empty) in
   * SERDATR */
  uint16_t out_word =
      (uint16_t)c | 0x0100; /* Add 9-bit stop bit parameters if configured */

  *(AMIGA_SERDAT) = out_word;
  printk("<7>[  %s ] Byte sent down hardware transmitter line: '%c' (0x%02X)\n",
         MODULE_NAME, c, c);
}

/**
 * amiserial_interrupt_handler: Triggered by the Paula chip line when a byte
 * hits the RX wire.
 */
void amiserial_interrupt_handler(void) {
  uint16_t data_reg = *(AMIGA_SERDATR);

  /* Check if bit 14 (RBF - Receive Buffer Full) is raised */
  if (data_reg & (1 << 14)) {
    char rx_char = (char)(data_reg & 0xFF);
    printk("<6>[  %s ] RX Intercept: Captured incoming byte: '%c'\n",
           MODULE_NAME, rx_char);

    /* Push raw token into tty line discipline tracking buffers */
  }
}

/**
 * amiserial_init: Setup routing bounds.
 */
void amiserial_init(void) {
  /* Set sample default period for 9600 baud rate allocation */
  *(AMIGA_SERPER) = 372;
  boot_msg(MODULE_NAME,
           "Custom Commodore Amiga hardware serialization interface active.",
           0);
}