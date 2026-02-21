void gpf_handler(registers_t regs) {
    k_panic("General Protection Fault (0x0D)");
}

void div_zero_handler(registers_t regs) {
    k_panic("Division by Zero Error (0x00)");
}