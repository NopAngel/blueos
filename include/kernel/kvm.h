#ifndef _BLUEOS_KVM_H
#define _BLUEOS_KVM_H

#include <stdint.h>

#define VMX_OK          0
#define VMX_ERROR       1

struct vmcs {
    uint32_t revision_id;
    uint32_t abort_indicator;
    uint8_t  data[4088];
} __attribute__((aligned(4096)));

struct vcpu {
    struct vmcs *vmcs_ptr;
    uint64_t vmxon_region_phys;
    int cpu_id;
};
int kvm_check_support(void);
int kvm_init_vmx(struct vcpu *vcpu);
void kvm_launch_guest(void);

#endif