// arch/i386/intel.c
#include <stdint.h>
#include <kernel/printk.h>
#include <kernel/colors.h>

#define IA32_FEATURE_CONTROL_MSR 0x3A


static inline uint64_t read_msr(uint32_t msr) {
    uint32_t low, high;
    asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((uint64_t)high << 32) | low;
}

static inline void write_msr(uint32_t msr, uint64_t val) {
    uint32_t low = (uint32_t)val;
    uint32_t high = (uint32_t)(val >> 32);
    asm volatile("wrmsr" : : "a"(low), "d"(high), "c"(msr));
}

int init_intel_vtx() {
    uint32_t eax, ebx, ecx, edx;

    asm volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0));
    if (ebx != 0x756e6547 || edx != 0x49656e69 || ecx != 0x6c65746e) {
        printk(YELLOW, "CPU: No Intel processor detected. Skipping VMX.\n");
        return 0;
    }


    asm volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(1));
    if (!(ecx & (1 << 5))) {
        printk(YELLOW, "Intel: This processor does not support VMX technology.\n");
        return 0;
    }

    uint64_t feature_control = read_msr(IA32_FEATURE_CONTROL_MSR);


    uint32_t msr_low, msr_high;
    asm volatile("rdmsr" : "=a"(msr_low), "=d"(msr_high) : "c"(IA32_FEATURE_CONTROL_MSR));

    if ((msr_low & 1) && !(msr_low & (1 << 2))) {
        printk(RED, "Intel: VMX is blocked by the BIOS (Disabled).\n");
        return 0;
    }


    uint32_t cr4;
    asm volatile("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= (1 << 13);
    asm volatile("mov %0, %%cr4" : : "r"(cr4));

    printk(GREEN, "Intel: Virtualization (VMX) successfully enabled.\n");
    return 1;
}

uint32_t _allocate_vmx_region() {
    return 0; //
}

void _init_vmcs(uint32_t* vmcs_ptr, uint32_t vmx_basic_low) {

    *vmcs_ptr = vmx_basic_low;

}

int setup_vmcs() {
    void* vmcs_ptr = _allocate_vmx_region();

    uint32_t vmx_basic_low, vmx_basic_high;
    asm volatile("rdmsr" : "=a"(vmx_basic_low), "=d"(vmx_basic_high) : "c"(0x480));

    _init_vmcs(vmcs_ptr, vmx_basic_low);

    uint8_t error;
    asm volatile (
        "vmptrld %[region];"
        "setna %[err]"
        : [err] "=rm" (error)
        : [region] "m" (vmcs_ptr)
        : "cc", "memory"
    );

    if (error) {
        printk(RED, "Intel: Error loading VMCS (VMPTRLD failure).\n");
        return -1;
    }

    printk(GREEN, "Intel: VMCS loaded and ready to configure interrupts.\n");
    return 0;
}
