struct bios_regs {
    uint16_t ax, bx, cx, dx;
    uint16_t si, di, bp;
    uint16_t ds, es, flags;
};

void bios_call(uint8_t interrupt, struct bios_regs *regs) {
}