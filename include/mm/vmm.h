#ifndef VMM_H
#define VMM_H

#include <stddef.h>
#include <stdint.h>

#define PAGE_SIZE 4096
#define PTE_V (1 << 0) // Valid
#define PTE_R (1 << 1) // Read
#define PTE_W (1 << 2) // Write
#define PTE_X (1 << 3) // Execute
#define PTE_U (1 << 4) // User
#define PTE_A (1 << 6) // Accessed
#define PTE_D (1 << 7) // Dirty

typedef uint32_t pte_t;
typedef pte_t paddr_t;
typedef uint32_t vaddr_t;

typedef struct {
  pte_t entries[1024];
} page_table_t;

void vmm_init();
void map_page(page_table_t *root, vaddr_t vaddr, paddr_t paddr, uint32_t flags);
void vmm_map(void *virtual, uint32_t physical, uint32_t size, uint32_t flags);
void *vmm_map_io(uintptr_t phys_addr, size_t size);
#endif
