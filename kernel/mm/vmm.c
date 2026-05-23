#include <mm/vmm.h>
#include <mm/pmm.h>
#include <kernel/malloc.h>
#include <kernel/colors.h>
#include <mm/memory.h>
#include <kernel/printk.h>
#include <lib/string.h>

#define IO_VIRT_OFFSET 0xFFFFFFC000000000
#define PAGE_PRESENT  0x1
#define PAGE_RW       0x2
#define PAGE_USER     0x4

uint32_t *current_page_directory = NULL;
static page_table_t *kernel_root_table = NULL;
extern char _end;

void vmm_init() {
    kernel_root_table = (page_table_t *)kmalloc(sizeof(page_table_t));
    memset(kernel_root_table, 0, PAGE_SIZE);

    for (uint32_t addr = 0x80000000; addr < (uint32_t)_end + (1024*1024); addr += PAGE_SIZE) {
        map_page(kernel_root_table, addr, addr, PTE_V | PTE_R | PTE_W | PTE_X);
    }

    map_page(kernel_root_table, 0x10000000, 0x10000000, PTE_V | PTE_R | PTE_W);

    printk(CYAN, "[VMM] Kernel and UART mapped 1:1. Ready to switch!\n");
}
void map_page(page_table_t *root, vaddr_t vaddr, paddr_t paddr, uint32_t flags) {
    uint32_t vpn1 = (vaddr >> 22) & 0x3FF;
    uint32_t vpn0 = (vaddr >> 12) & 0x3FF;

    if (!(root->entries[vpn1] & PTE_V)) {
        page_table_t *new_table = (page_table_t *)kmalloc(sizeof(page_table_t));
        memset(new_table, 0, PAGE_SIZE);
        root->entries[vpn1] = (((uintptr_t)new_table >> 12) << 10) | PTE_V;
    }

    page_table_t *leaf_table = (page_table_t *)(((root->entries[vpn1] >> 10) << 12));
    leaf_table->entries[vpn0] = ((paddr >> 12) << 10) | flags | PTE_V;
}
void vaddr_switch_to_kernel() {
    extern void enable_paging(uintptr_t root_table_addr);

    enable_paging((uintptr_t)kernel_root_table);
}

void* vmm_map_io(uintptr_t phys_addr, size_t size) {
    uintptr_t virt_addr = phys_addr + IO_VIRT_OFFSET;

    /*
     * map_page(root_page_table, virt_addr, phys_addr, PAGE_READ | PAGE_WRITE | PAGE_NOCACHE);
     */

    return (void*)virt_addr;
}

int vmm_map_page(uintptr_t vaddr, uintptr_t paddr, uint32_t flags) {
    /* 1. Get the Page Directory Entry (PDE) index */
    /* Virtual address bits 22-31 point to the PDE index */
    uint32_t pd_index = vaddr >> 22;

    /* 2. Get the Page Table Entry (PTE) index */
    /* Virtual address bits 12-21 point to the PTE index */
    uint32_t pt_index = (vaddr >> 12) & 0x3FF;

    /* Get the current Page Directory (this is usually a global pointer in your VMM) */
    uint32_t pde = current_page_directory[pd_index];

    uint32_t *page_table;

    /* 3. Check if the Page Table already exists */
    if (!(pde & PAGE_PRESENT)) {
        /* The Page Table doesn't exist, we must allocate a new one! */
        page_table = (uint32_t *)pmm_alloc_frame(); // Using your PMM
        if (!page_table) {
            return -1; // Out of physical memory for the PT
        }

        /* Zero out the new page table to avoid garbage mappings */
        mm_memset(page_table, 0, PAGE_SIZE);

        /* Map the new Page Table into the Directory */
        /* We use flags 0x7 so the directory allows the table's specific permissions */
        current_page_directory[pd_index] = (uint32_t)page_table | PAGE_PRESENT | PAGE_RW | PAGE_USER;
    } else {
        /* The table exists, just extract its physical address (mask out the flags) */
        page_table = (uint32_t *)(pde & ~0xFFF);
    }

    /* 4. Map the physical address into the Page Table */
    /* Ensure the physical address is aligned and apply flags */
    page_table[pt_index] = (paddr & ~0xFFF) | flags;

    /* 5. Invalidate the TLB (Translation Lookaside Buffer) */
    /* This tells the CPU that the mapping has changed */
#if defined(x86)
    asm volatile("invlpg (%0)" : : "r" (vaddr) : "memory");
#elif defined(__riscv)
    asm volatile("sfence.vma %0" : : "r" (vaddr) : "memory");
#endif

    return 0; // Success!
}
