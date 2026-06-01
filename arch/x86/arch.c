#include <arch/x86/gdt.h>
#include <arch/x86/idt.h>
#include <arch/x86/pic.h>
#include <kernel/arch.h>
#include <kernel/ports.h>
#include <kernel/printk.h>
#include <stdbool.h>
#include <version.h>

static void arch_verify_identity(void) {
  uint32_t eax, ebx, ecx, edx;
  /* Ejecutamos CPUID leaf 0 para obtener el Vendor ID */
  asm volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0));

  /* Intel devuelve "Genu" (0x756e6547) y AMD devuelve "Auth" (0x68747541) en
   * EBX */
  bool is_x86 = (ebx == 0x756e6547 || ebx == 0x68747541);

  if (!is_x86) {
    printk(
        "\n\033[31;1m[ FATAL ERROR ] Architecture Mismatch Detected!\033[0m\n");
    printk("This BlueOS kernel was compiled for: \033[33m%s\033[0m\n",
           BLUEOS_ARCH);
    printk(
        "Current hardware identity check failed. You might be running this\n");
    printk("x86 image on a different architecture emulator (like RISC-V or "
           "ARM).\n");
    printk("\nSystem Halted to prevent corruption.\n");
    while (1)
      asm volatile("cli; hlt");
  }
}

void arch_early_init(void) {
  arch_verify_identity();
  gdt_init();
  idt_init();

  pic_init(0x20, 0x28);

  boot_msg("ARCH", "x86 (x86) early initialization complete.\n", 0);
}

void arch_init(void) { arch_early_init(); }

void arch_disable_interrupts(void) { asm volatile("cli"); }

void arch_enable_interrupts(void) { asm volatile("sti"); }

void arch_idle(void) { asm volatile("hlt"); }

bool arch_is_guest(void) {
  extern bool detect_hypervisor(void);
  return detect_hypervisor();
}

const char *arch_get_hypervisor_name(void) {
  extern const char *x86_hyper_name(void);
  return x86_hyper_name();
}

void *arch_prepare_stack(void *stack_top, void (*fn)(void)) {
  uint32_t *stack = (uint32_t *)stack_top;

  *(--stack) = (uint32_t)fn; /* EIP */
  *(--stack) = 0;            /* EBP */
  *(--stack) = 0;            /* EDI */
  *(--stack) = 0;            /* ESI */
  *(--stack) = 0;            /* EBX */
  *(--stack) = 0;            /* EDX */
  *(--stack) = 0;            /* ECX */
  *(--stack) = 0;            /* EAX */

  *(--stack) = 0x10; /* DS */
  *(--stack) = 0x10; /* ES */
  *(--stack) = 0x10; /* FS */
  *(--stack) = 0x10; /* GS */

  return (void *)stack;
}