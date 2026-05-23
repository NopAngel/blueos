#ifndef KVM_HOST_H
#define KVM_HOST_H

#include <stdint.h>
#include <drivers/virtio_net.h>

/* Forward declarations to avoid circular dependencies */
struct kvm;
struct kvm_riscv_nmu;

struct kvm_vcpu_arch {
    uintptr_t gprs[32];   /* General Purpose Registers */
    uintptr_t vsatp;      /* Guest Stage-1 Page Table */
    uintptr_t hstatus;    /* Hypervisor Status */
};

struct kvm_vcpu {
    struct kvm *kvm;      /* Pointer to the parent VM instance */
    int vcpu_id;
    struct kvm_vcpu_arch arch;
};

struct kvm_arch {
    struct kvm_riscv_nmu *nmu; /* Network Management Unit instance */
};

struct kvm {
    struct kvm_arch arch;
};

#endif