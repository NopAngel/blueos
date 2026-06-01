#include <stdint.h>
#include <kernel/printk.h>
#include <kernel/colors.h>

/* CSR Addresses */
#define CSR_MVENDORID 0xf11
#define CSR_MISA      0x301

static inline uintptr_t read_csr(int csr_num) {
    uintptr_t val;
    if (csr_num == CSR_MVENDORID) asm volatile("csrr %0, mvendorid" : "=r"(val));
    else if (csr_num == CSR_MISA) asm volatile("csrr %0, misa" : "=r"(val));
    return val;
}

int init_riscv_hypervisor() {
    uintptr_t vendor = read_csr(CSR_MVENDORID);
    uintptr_t misa = read_csr(CSR_MISA);

    if (vendor == 0) {
        printk("CPU: Non-commercial or Open Source Vendor ID.\n");
    } else {
        printk("CPU: Vendor ID: 0x%lx\n", vendor);
    }

    if (!(misa & (1 << 7))) {
        printk("RISC-V: This CPU does not support Hypervisor Extension (H).\n");
        return 0;
    }

    printk("RISC-V: Virtualization support (H-Extension) detected and ready.\n");
    return 1;
}
