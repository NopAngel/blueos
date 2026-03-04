#include <stdint.h>
#include <blueos/printk.h>
#include <blueos/colors.h>

int init_amd_svm() {
    uint32_t eax, ebx, ecx, edx;

    asm volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0));
    if (ebx != 0x68747541 || edx != 0x69746e65 || ecx != 0x444d4163) { // "Auth" "enti" "cAMD"
        printk(YELLOW, "CPU: No se detecto un procesador AMD.\n");
        return 0;
    }

    asm volatile("cpuid" : "=a"(eax) : "a"(0x80000000));
    if (eax < 0x80000001) {
        return 0; 
    }

    asm volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0x80000001));
    if (!(ecx & (1 << 2))) {
        printk(YELLOW, "AMD: Este procesador no soporta tecnologia SVM (AMD-V).\n");
        return 0;
    }

    uint32_t efer_low, efer_high;
    asm volatile("rdmsr" : "=a"(efer_low), "=d"(efer_high) : "c"(0xC0000080));
    
    efer_low |= (1 << 12); // SVME
    
    asm volatile("wrmsr" : : "a"(efer_low), "d"(efer_high), "c"(0xC0000080));

    printk(GREEN, "AMD: Virtualizacion (SVM) habilitada correctamente.\n");
    return 1;
}