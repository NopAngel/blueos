#include <stdbool.h>
#include <stdbool.h>

void arch_idle() { asm volatile("hlt"); }
void arch_early_init() { riscv_setup_traps(); plic_init(); }
bool arch_is_guest() { return false; } 
const char* arch_get_hypervisor_name() { return "QEMU Virt"; }

void* arch_prepare_stack(void* stack_top, void (*fn)(void)) {
    uintptr_t* stack = (uintptr_t*)stack_top;

    *(--stack) = (uintptr_t)fn;   /* EPC / RA:  */
    
    for (int i = 0; i < 30; i++) {
        *(--stack) = 0;           /*  0 */
    }

    return (void*)stack;
}