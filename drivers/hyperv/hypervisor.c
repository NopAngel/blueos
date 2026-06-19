#include <kernel/printk.h>
#include <stdint.h>

void detect_hypervisor() {
#if defined(__x86__) || defined(x86)
  uint32_t eax, ebx, ecx, edx;
  uint32_t leaf = 0x40000000;

  __asm__ volatile("cpuid"
                   : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                   : "a"(leaf));

  char signature[13];
  *((uint32_t *)(signature + 0)) = ebx;
  *((uint32_t *)(signature + 4)) = ecx;
  *((uint32_t *)(signature + 8)) = edx;
  signature[12] = '\0';

  printk("<6> [CPUID] Hypervisor Signature: %s\n", signature);

  if (ebx == 0x4b4d564b) { // "KVMK"
    printk("<6> [INFO] BlueOS running on KVM (Hardware Accelerated)\n");
  } else if (ebx == 0x47435447) { // "GCTG" (parte de TCGTCG en little-endian)
    printk("<6> [INFO] BlueOS running on QEMU TCG (Software Emulated)\n");
  } else {
    printk("<6> [INFO] Hypervisor unknown: %s\n", signature);
  }
#endif
}
