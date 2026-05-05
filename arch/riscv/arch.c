void arch_putc(char c) {
    volatile char *uart = (char *)0x10000000;
    *uart = c;
}
