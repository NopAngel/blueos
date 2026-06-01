#include <stdint.h>
#include <kernel/printk.h>
#include <kernel/colors.h>

int init_amd_svm() {
    uint32_t eax, ebx, ecx, edx;

    asm volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0));
    if (ebx != 0x68747541 || edx != 0x69746e65 || ecx != 0x444d4163) { // "Auth" "enti" "cAMD"
        printk("CPU: No AMD processor detected.\n");
        return 0;
    }

    asm volatile("cpuid" : "=a"(eax) : "a"(0x80000000));
    if (eax < 0x80000001) {
        return 0;
    }

    asm volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0x80000001));
    if (!(ecx & (1 << 2))) {
        printk("AMD: This processor does not support SVM technology (AMD-V).\n");
        return 0;
    }

    uint32_t efer_low, efer_high;
    asm volatile("rdmsr" : "=a"(efer_low), "=d"(efer_high) : "c"(0xC0000080));

    efer_low |= (1 << 12); // SVME

    asm volatile("wrmsr" : : "a"(efer_low), "d"(efer_high), "c"(0xC0000080));

    printk("AMD: Virtualization (SVM) successfully enabled.\n");
    return 1;
}
