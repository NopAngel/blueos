void arch_putc(char c) {
    // En RISC-V (QEMU virt), escribimos en un puerto UART
    volatile char *uart = (char *)0x10000000;
    *uart = c;
}