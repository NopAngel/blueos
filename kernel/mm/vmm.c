#include <mm/vmm.h>
#include <kernel/malloc.h>
#include <kernel/colors.h>
#include <kernel/printk.h>
#include <lib/string.h>

#define IO_VIRT_OFFSET 0xFFFFFFC000000000

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