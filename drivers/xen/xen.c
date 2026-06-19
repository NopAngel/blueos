#include <drivers/xen.h>
#include <lib/string.h>

void xen_console_write(const char *str) {
  int len = strlen(str);
  xen_hypercall(__HYPERVISOR_console_io, 0, len, (long)str);
}

void xen_reboot() { xen_hypercall(__HYPERVISOR_sched_op, 0, 1, 0); }

int is_xen_present() {
#if defined(__x86__)
  uint32_t eax, ebx, ecx, edx;
  asm volatile("cpuid"
               : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
               : "a"(0x40000000));
  return (ebx == 0x566e6558); // "XenV"
#elif defined(__riscv)
  return 1;
#endif
}