#include <stdbool.h>
#include <stdint.h>

static bool hyper_is_qemu = false;
static bool hyper_is_guest = false;
static const char* hyper_name = "Bare Metal";


void detect_hypervisor(void) {
    uint32_t eax, ebx, ecx, edx;
    
    // Check for Hypervisor presence bit in CPUID
    __asm__ volatile ("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(1));
    if (!(ecx & (1 << 31))) return; // Bit 31 of ECX means "Hypervisor present"

    // Get Hypervisor signature
    __asm__ volatile ("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0x40000000));

    hyper_is_guest = true;

    // Check common signatures
    if (ebx == 0x4b4d564b) hyper_name = "KVM/QEMU";          // "KVMK"
    else if (ebx == 0x564d7761) hyper_name = "VMware";      // "VMwa"
    else if (ebx == 0x7262764c) hyper_name = "VirtualBox";  // "VBox"
    else hyper_name = "Unknown Hypervisor";
}
bool x86_is_guest(void) { return hyper_is_qemu; }

const char* x86_hyper_name(void) {
    return hyper_is_qemu ? "QEMU/KVM" : "Bare Metal";
}