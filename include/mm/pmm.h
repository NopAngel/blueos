#ifndef PMM_H
#define PMM_H

void pmm_set_bit(uint32_t bit);
void pmm_unset_bit(uint32_t bit);
int pmm_test_bit(uint32_t bit);

void pmm_init(uint32_t mem_size, uint32_t bitmap_addr);

void pmm_init_region(uint32_t base, uint32_t size);

int pmm_find_first_free();
void *kalloc_page();
void *pmm_alloc_block();
void pmm_free_block(void *addr);
void *pmm_alloc_frame();
void pmm_free_frame(void *paddr);
#endif
