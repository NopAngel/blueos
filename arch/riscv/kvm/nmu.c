#include <asm/kvm_host.h>
#include <kernel/net.h>
#include <drivers/virtio_net.h>
#include <stddef.h>
#include <mm/memory.h>
#include <lib/string.h>

/**
 * struct kvm_riscv_nmu - Represents the Network Management Unit for a Guest
 * @doorbell_addr: Guest physical address used to trigger TX/RX processing
 * @config: Virtual NIC configuration (MAC, MTU, etc.)
 */
struct kvm_riscv_nmu {
    uintptr_t doorbell_addr;
    struct virtio_net_config config;
};

/**
 * kvm_riscv_gpa_to_hpa - Translate Guest Physical Address to Host Physical Address
 * @gpa: Guest address
 * Returns: Actual physical address in the Host RAM
 */
static uintptr_t kvm_riscv_gpa_to_hpa(uintptr_t gpa) {
    /* TODO: Implement Stage-2 Page Table walking (HGATP)
     * For now, we assume a direct identity mapping for testing */
    return gpa;
}

/**
 * nmu_process_tx_queue - Handle packet transmission from the Guest
 * @vcpu: The virtual CPU that triggered the kick
 * * This function reads the VirtIO descriptors from Guest memory,
 * allocates a network buffer in BlueOS, and transmits the frame.
 */
void nmu_process_tx_queue(struct kvm_vcpu *vcpu) {
    /* 1. Get NMU context from the VM architecture */
    struct kvm_riscv_nmu *nmu = vcpu->kvm->arch.nmu;

    /* 2. Logic to fetch VirtIO descriptors (simplified)
     * In a real implementation, we would parse the VRing available ring here */
    uintptr_t guest_packet_addr = 0x81000000; // Example GPA
    uint32_t packet_len = 1500;

    /* 3. Translate GPA to Host Physical Address */
    uintptr_t hpa = kvm_riscv_gpa_to_hpa(guest_packet_addr);

    /* 4. Allocate a packet in BlueOS Network Stack */
    struct net_packet *pkt = net_alloc_packet();
    if (!pkt) return;

    /* 5. Zero-copy / Copy data from Guest memory to Host network buffer */
    memcpy(pkt->data, (void*)hpa, packet_len);
    pkt->len = packet_len;

    /* 6. Send the packet through the physical NIC */
    net_transmit(pkt);
}

/**
 * kvm_riscv_nmu_kick - Entry point for NMU acceleration
 * @vcpu: Virtual CPU causing the trap
 * @addr: The address being accessed (MMIO Doorbell)
 * * Triggered when the Guest writes to the network doorbell.
 */
void kvm_riscv_nmu_kick(struct kvm_vcpu *vcpu, uintptr_t addr) {
    struct kvm_riscv_nmu *nmu = vcpu->kvm->arch.nmu;

    if (!nmu) return;

    /* Check if the trap address matches the NMU doorbell */
    if (addr == nmu->doorbell_addr) {
        /* Process the TX queue to send data out */
        nmu_process_tx_queue(vcpu);
    }
}

/**
 * kvm_riscv_nmu_init - Initialize a new NMU instance for a VM
 */
struct kvm_riscv_nmu* kvm_riscv_nmu_init(uintptr_t doorbell) {
    struct kvm_riscv_nmu *nmu = (struct kvm_riscv_nmu*)kmalloc(sizeof(struct kvm_riscv_nmu));
    if (!nmu) return NULL;

    nmu->doorbell_addr = doorbell;

    /* Default Virtual MAC for the Guest */
    nmu->config.mac[0] = 0x02;
    nmu->config.mac[1] = 0x00;
    nmu->config.mac[2] = 0x42; /* B */
    nmu->config.mac[3] = 0x4C; /* L */
    nmu->config.mac[4] = 0x55; /* U */
    nmu->config.mac[5] = 0x45; /* E */

    return nmu;
}
