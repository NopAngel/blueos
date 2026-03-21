void disable_interrupts() {
    asm volatile ("csrci mstatus, 8");
}