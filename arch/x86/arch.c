#include <stdbool.h>
#include <kernel/arch.h>
#include <arch/x86/gdt.h>
#include <arch/x86/idt.h>
#include <arch/x86/pic.h>
#include <kernel/ports.h>
#include <kernel/printk.h>


void arch_early_init(void) {
    gdt_init();
    idt_init();
    
    pic_init(0x20, 0x28);

    pr_info("ARCH: x86 (x86) early initialization complete.\n");
}


void arch_init(void) {
    arch_early_init();
}

void arch_disable_interrupts(void) {
    asm volatile("cli");
}

void arch_enable_interrupts(void) {
    asm volatile("sti");
}

void arch_idle(void) {
    asm volatile("hlt");
}


bool arch_is_guest(void) {
    extern bool detect_hypervisor(void);
    return detect_hypervisor();
}


const char* arch_get_hypervisor_name(void) {
    extern const char* x86_hyper_name(void);
    return x86_hyper_name();
}

void* arch_prepare_stack(void* stack_top, void (*fn)(void)) {
    uint32_t* stack = (uint32_t*)stack_top;

    *(--stack) = (uint32_t)fn;    /* EIP */
    *(--stack) = 0;               /* EBP */
    *(--stack) = 0;               /* EDI */
    *(--stack) = 0;               /* ESI */
    *(--stack) = 0;               /* EBX */
    *(--stack) = 0;               /* EDX */
    *(--stack) = 0;               /* ECX */
    *(--stack) = 0;               /* EAX */
    
    *(--stack) = 0x10;            /* DS */
    *(--stack) = 0x10;            /* ES */
    *(--stack) = 0x10;            /* FS */
    *(--stack) = 0x10;            /* GS */

    return (void*)stack;
}