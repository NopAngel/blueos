#include <drivers/keyboard.h>
#include <stdint.h>

/* --- UART0 Registers (QEMU Virt Machine) --- */
#define UART0_BASE 0x10000000
#define UART_DR    ((volatile uint8_t *)(UART0_BASE + 0)) // Data Register
#define UART_LSR   ((volatile uint8_t *)(UART0_BASE + 5)) // Line Status Register
#define UART_LSR_EMPTY 0x20

/**
 * uart_put_char: Internal helper to send a byte to the serial port.
 */
static void uart_put_char(char c) {
    /* Wait for the Transmit Holding Register to be empty */
    while (!(*UART_LSR & UART_LSR_EMPTY));
    *UART_DR = (uint8_t)c;
}
