#include <stdint.h>
#include <mm/memory.h>      
#include <kernel/printk.h>
#include <kernel/colors.h>

#define CSR_HGATP   0x680  
#define CSR_HSTATUS 0x600  

int init_riscv_vtx_compat() {
    uintptr_t misa;
    asm volatile("csrr %0, misa" : "=r"(misa));

    if (!(misa & (1 << 7))) {
        printk(YELLOW, "RISC-V: Hypervisor extension (H) not supported.\n");
        return 0; 
    }

    uintptr_t hstatus;
    asm volatile("csrr %0, hstatus" : "=r"(hstatus));
    asm volatile("csrw hstatus, %0" : : "r"(hstatus));

    printk(GREEN, "RISC-V: Hypervisor (H) enabled (Pure C Mode).\n");
    return 1; 
}


int setup_guest_context() {
    void* guest_pt_ptr = kmalloc(4096); 
    
    if (!guest_pt_ptr) {
        printk(RED, "RISC-V: Error: Out of memory for Guest Tables.\n");
        return -1;
    }

    mm_memset(guest_pt_ptr, 0, 4096);


    uintptr_t hgatp_val = ((uintptr_t)guest_pt_ptr >> 12); 
    
    #if __riscv_xlen == 32
        hgatp_val |= (1U << 31); // mode Sv32
    #else
        hgatp_val |= (8ULL << 60); // mode Sv39 for 64-bit
    #endif

    asm volatile ("csrw hgatp, %0" : : "r"(hgatp_val));

    asm volatile (
        ".option push\n"
        ".option arch, +h\n"
        "hfence.gvma\n"
        ".option pop"
        : : : "memory"
    );

    printk(GREEN, "RISC-V: Guest Stage-2 Tables allocated at 0x%lx\n", (uintptr_t)guest_pt_ptr);
    return 0;
}