#include <stdint.h>

/* UART 16550a Base Address (QEMU Virt Machine standard) */
#define UART_BASE 0x10000000

/* Register Offsets */
#define UART_RHR (uint8_t*)(UART_BASE + 0) // Receive Holding Reg (Read)
#define UART_THR (uint8_t*)(UART_BASE + 0) // Transmit Holding Reg (Write)
#define UART_LSR (uint8_t*)(UART_BASE + 5) // Line Status Reg

/* Line Status Register (LSR) Bits */
#define LSR_RX_READY 0x01 // Bit 0: Data ready to be read
#define LSR_TX_IDLE  0x20 // Bit 5: THR empty, ready to transmit

/**
 * arch_get_char - Reads a character from UART if available
 */
int arch_get_char() {
    /* Check if the 'Data Ready' bit is set in LSR */
    if (*UART_LSR & LSR_RX_READY) {
        return *UART_RHR;
    }
    return -1; // No data available
}

/**
 * arch_put_char - Sends a character through UART
 */
void arch_put_char(char c) {
    /* Wait until the 'Transmit Holding Register Empty' bit is set */
    while ((*UART_LSR & LSR_TX_IDLE) == 0);
    *UART_THR = c;
}