#include <stdint.h>
#include <kernel/sched.h>
#include <kernel/process.h>
#include <kernel/printk.h>
#include <mm/memory.h> // Using our fixed memory manager

/* PMM: Physical Memory Manager - Allocates a 4KB frame */
extern void* pmm_alloc_frame();
/* VMM: Virtual Memory Manager - Maps VIRT to PHYS in current Page Table */
extern int vmm_map_page(uintptr_t vaddr, uintptr_t paddr, uint32_t flags);

/**
 * handle_demand_paging - Resolves a Page Fault by allocating memory on-the-fly.
 * @task: The process that triggered the fault.
 * @addr: The virtual address that caused the exception (CR2 on x86, stval on RISC-V).
 *
 * Returns: 1 on success (retry instruction), 0 on segmentation fault.
 */
int handle_demand_paging(struct task_struct *task, uintptr_t addr) {
    /* 1. Page Alignment (Crucial) */
    /* Virtual addresses can be anywhere, but we must map the START of the page (4KB aligned) */
    uintptr_t aligned_addr = addr & ~(PAGE_SIZE - 1);

    /* 2. Security Check (Basic VMA validation) */
    /* In a real kernel, you'd check task->vma_list to see if this address is 'owned' */
    if (addr < 0x1000) {
        pr_err("Segmentation Fault: Access to NULL or reserved area at 0x%p\n", addr);
        return 0;
    }

    /* 3. Allocate Physical Frame */
    /* Using the PMM we discussed before */
    void *phys_frame = pmm_alloc_frame();
    if (!phys_frame) {
        pr_err("Out of memory: Swap or OOM-Killer needed for %s\n", task->name);
        return 0;
    }

    /* 4. Zero out the new page (Security: don't leak other processes' data) */
    mm_memset(phys_frame, 0, PAGE_SIZE);

    /* 5. Map the page into the Page Table */
    /* Flags 0x7: Present (1) | Read-Write (2) | User-mode (4) */
    if (vmm_map_page(aligned_addr, (uintptr_t)phys_frame, 0x7) != 0) {
        // If mapping fails, free the frame to avoid leaks
        // pmm_free_frame(phys_frame);
        return 0;
    }

    pr_info("[MMU] Demand Paging: Mapped 0x%p -> 0x%p for %s\n",
            aligned_addr, (uintptr_t)phys_frame, task->name);

    return 1; // Success! The CPU will now retry the instruction that faulted.
}
