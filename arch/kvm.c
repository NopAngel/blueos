#include <blueos/kvm.h>
#include <blueos/io.h>
#include <blueos/printk.h>
#include <blueos/colors.h>

#define MSR_IA32_FEATURE_CONTROL    0x0000003A

void kvm_unlock_vmx() {
    uint32_t lo, hi;
    asm volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(MSR_IA32_FEATURE_CONTROL));

    if (!(lo & 1)) {
        lo |= (1 << 2) | (1 << 0);
        asm volatile("wrmsr" : : "a"(lo), "d"(hi), "c"(MSR_IA32_FEATURE_CONTROL));
        printk(GREEN, "KVM: Feature Control MSR unlocked and VMX enabled.\n");
    }
}

int kvm_check_support(void) {
    uint32_t eax, ebx, ecx, edx;
    
    asm volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(1));
    
    if (ecx & (1 << 5)) {
        printk(GREEN, "KVM: Intel VT-x supported!\n");
        return VMX_OK;
    }
    return VMX_ERROR;
}

int kvm_init_vmx(struct vcpu *vcpu) {
    uint64_t cr4;
    
    
    asm volatile("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= (1 << 13);
    asm volatile("mov %0, %%cr4" :: "r"(cr4));

    // asm volatile("vmxon (%0)" : : "r"(&vcpu->vmxon_region_phys) : "cc", "memory");

    printk(GREEN, "KVM: VMX root operation enabled.\n");
    return VMX_OK;
}