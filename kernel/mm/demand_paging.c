#include <stdint.h>
#include <kernel/process.h>
#include <kernel/printk.h>

/* This is a mock-up of a page allocator. 
 * In BlueOS, this should call your physical memory manager (PMM) */
extern void* kalloc_page(); 
extern int map_page(uintptr_t vaddr, uintptr_t paddr, int flags);

int handle_demand_paging(struct task_struct *task, uintptr_t addr) {
    /* 1. Check if the address is within a valid memory region of the process */
    /* (For now, let's assume any address > 0x1000 is valid for testing) */
    if (addr < 0x1000) return 0; 

    /* 2. Allocate a real physical frame (4KB) */
    void *phys_page = kalloc_page();
    if (!phys_page) {
        printk(RED, "Out of memory! Cannot satisfy page fault.\n");
        return 0;
    }

    /* 3. Map the virtual address to the new physical page in the process page table */
    /* Flags: Read/Write/User (0x7) */
    map_page(addr, (uintptr_t)phys_page, 0x7);

    printk(GREEN, "[MMU] Allocated new page at 0x%x for %s\n", addr, task->name);
    return 1; // Success! The CPU will retry the instruction.
}