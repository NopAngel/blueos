#include <stdint.h>
#include <kernel/printk.h>
#include <kernel/colors.h>

#define CSR_MISA    0x301
#define CSR_HSTATUS 0x600
#define CSR_HEDELEG 0x602  
#define CSR_HIDELEG 0x603  
static inline uintptr_t csr_read(int csr_num) {
    uintptr_t val;
    if (csr_num == CSR_MISA) asm volatile("csrr %0, misa" : "=r"(val));
    else if (csr_num == CSR_HSTATUS) asm volatile("csrr %0, hstatus" : "=r"(val));
    return val;
}

static inline void csr_write(int csr_num, uintptr_t val) {
    if (csr_num == CSR_HSTATUS) asm volatile("csrw hstatus, %0" :: "r"(val));
    else if (csr_num == CSR_HEDELEG) asm volatile("csrw hedeleg, %0" :: "r"(val));
    else if (csr_num == CSR_HIDELEG) asm volatile("csrw hideleg, %0" :: "r"(val));
}


int kvm_check_support(void) {
    uintptr_t misa = csr_read(CSR_MISA);
    
    if (misa & (1 << 7)) {
        printk(GREEN, "KVM: RISC-V Hypervisor Extension (H) supported!\n");
        return 0; // OK
    }
    
    printk(RED, "KVM: Hypervisor Extension NOT found.\n");
    return -1;
}


int kvm_init_hypervisor(void) {
    if (kvm_check_support() != 0) return -1;

    csr_write(CSR_HEDELEG, 0xFFFF); 
    csr_write(CSR_HIDELEG, 0xFFFF);

    uintptr_t hstatus = csr_read(CSR_HSTATUS);
    csr_write(CSR_HSTATUS, hstatus);

    printk(GREEN, "KVM: Hypervisor mode initialized (CSRs delegated).\n");
    return 0;
}

void kvm_print_info() {
    uintptr_t misa = csr_read(CSR_MISA);
    printk(CYAN, "KVM: MISA ISA String: ");
    for (int i = 0; i < 26; i++) {
        if (misa & (1 << i)) {
            char feat = 'A' + i;
            printk(WHITE, "%c", feat);
        }
    }
    printk(WHITE, "\n");
}