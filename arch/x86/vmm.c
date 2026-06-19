#include <kernel/printk.h>
#include <kernel/spinlock.h>
#include <lib/string.h>
#include <mm/memory.h>

#define PAGE_PRESENT 0x01
#define PAGE_RW 0x02
#define PAGE_USER 0x04
#define PAGE_COW 0x200 // Bit 9: Custom COW bit

extern void *pmm_alloc_frame();

static spinlock_t vmm_lock = SPINLOCK_INIT;

/**
 * vmm_copy_page: Realiza la copia física de una página para COW
 */
void vmm_handle_cow(uint32_t fault_addr, uint32_t *pte) {
  spin_lock(&vmm_lock);

  uint32_t old_page_phys = *pte & 0xFFFFF000;
  uint32_t new_page_phys = (uint32_t)pmm_alloc_frame();

  memcpy((void *)new_page_phys, (void *)(fault_addr & 0xFFFFF000), 4096);

  *pte = new_page_phys | PAGE_PRESENT | PAGE_RW | PAGE_USER;

  __asm__ volatile("invlpg (%0)" ::"r"(fault_addr) : "memory");

  spin_unlock(&vmm_lock);
  printk("\033[32mVMM: COW handled for address 0x%x\033[0m\n", fault_addr);
}

/**
 * vmm_handle_demand_paging: Carga una página que no estaba en RAM
 */
void vmm_handle_demand_paging(uint32_t fault_addr, uint32_t *pte) {
  spin_lock(&vmm_lock);

  uint32_t new_page = (uint32_t)pmm_alloc_frame();
  memset((void *)new_page, 0, 4096); // Zero-fill on demand

  *pte = new_page | PAGE_PRESENT | PAGE_RW | PAGE_USER;

  __asm__ volatile("invlpg (%0)" ::"r"(fault_addr) : "memory");

  spin_unlock(&vmm_lock);
  printk("\033[32mVMM: Demand Paging: Zero-filled page at 0x%x\033[0m\n",
         fault_addr);
}

void vmm_map(void *virtual, uint32_t physical, uint32_t size, uint32_t flags) {
  spin_lock(&vmm_lock);

  uint32_t addr = (uint32_t)virtual;
  uint32_t end = addr + size;

  while (addr < end) {

    uint32_t pd_index = addr >> 22;
    uint32_t pt_index = (addr >> 12) & 0x3FF;

    uint32_t *pd = (uint32_t *)0xFFFFF000;

    if (!(pd[pd_index] & PAGE_PRESENT)) {
      uint32_t new_pt = (uint32_t)pmm_alloc_frame();
      pd[pd_index] = new_pt | PAGE_PRESENT | PAGE_RW | PAGE_USER;
      memset((void *)(new_pt), 0, 4096);
    }

    uint32_t *pt = (uint32_t *)((pd[pd_index] & 0xFFFFF000));
    pt[pt_index] = physical | flags;

    __asm__ volatile("invlpg (%0)" ::"r"(addr) : "memory");

    addr += 4096;
    physical += 4096;
  }

  spin_unlock(&vmm_lock);
}
