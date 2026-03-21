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

/**
 * arch_put_char: Implementation of the keyboard's visual output.
 */
void arch_put_char(char c, unsigned int color) {
    /* 1. Handle Backspace (ANSI Sequence) */
    if (c == '\b') {
        /* Move cursor back, overwrite with space, move back again */
        uart_put_char('\b');
        uart_put_char(' ');
        uart_put_char('\b');
        return;
    }

    /* 2. Output the character */
    uart_put_char(c);

    /* 3. Handle Newline: Add Carriage Return (\r) 
       This is what prevents the prompt from multiplying in the same line! */
    if (c == '\n') {
        uart_put_char('\r');
    }
}

/**
 * arch_get_scancode: Reads a byte from the UART if available.
 */
uint8_t arch_get_scancode(void) {
    /* Check if the Receiver Data Ready bit is set */
    if (!(*UART_LSR & 0x01)) {
        return 0; // Nothing to read
    }
    return *UART_DR;
}