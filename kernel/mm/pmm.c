#include <stdint.h>
#include <kernel/printk.h>

#define PAGE_SIZE 4096
#define MAX_PAGES 1024  
static uint8_t page_bitmap[MAX_PAGES];
extern uintptr_t _end; 

void* kalloc_page() {
    for (int i = 0; i < MAX_PAGES; i++) {
        if (page_bitmap[i] == 0) {
            page_bitmap[i] = 1;
            

            uintptr_t phys_addr = (uintptr_t)&_end + (i * PAGE_SIZE);

            for (int b = 0; b < PAGE_SIZE; b++) {
                ((char*)phys_addr)[b] = 0;
            }
            
            return (void*)phys_addr;
        }
    }
    return (void*)0; // Out of memory!
}

void kfree_page(void* ptr) {
    uintptr_t addr = (uintptr_t)ptr;
    uintptr_t base = (uintptr_t)&_end;
    
    int index = (addr - base) / PAGE_SIZE;
    if (index >= 0 && index < MAX_PAGES) {
        page_bitmap[index] = 0;
    }
}