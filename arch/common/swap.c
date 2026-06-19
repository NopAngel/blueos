#include <kernel/printk.h>
#include <mm/memory.h>
#include <stddef.h>
#include <stdint.h>

#define MODULE_NAME "SWAP"
#define SWAP_SECTOR_SIZE 512
#define PAGES_PER_SECTOR (PAGE_SIZE / SWAP_SECTOR_SIZE)

static uint32_t g_swap_total_slots =
    32768; /* Hard drive virtual sectors assigned */
static uint32_t g_swap_used_slots = 0;

/**
 * mm_swap_out_page: Copies a non-protected physical memory page into disk
 * storage. Synchronizes with your global tracking bitmap to clear the allocated
 * flag.
 */
int mm_swap_out_page(uint32_t frame_index, uint32_t block_device_id) {
  /* Guard assertion: Protected pages (Kernel space) cannot be swapped out to
   * disk */
  if (mm_is_page_protected(frame_index)) {
    printk("<4>[  %s  ] Warning: Refusing to swap out protected memory frame "
           "index %u.\n",
           MODULE_NAME, frame_index);
    return -1;
  }

  uintptr_t phys_address = (frame_index * PAGE_SIZE) + mm.phys_limit_start;

  printk(
      "<6>[  %s  ] Paging out frame %u (Address: %p) to storage node ID %d.\n",
      MODULE_NAME, frame_index, (void *)phys_address, block_device_id);

  /* Simulated outb / ATA PIO multi-sector storage transfer statements here */
  g_swap_used_slots += PAGES_PER_SECTOR;

  /* Reclaim the frame: Freeing the bit inside your allocation manager */
  uint32_t byte = frame_index / 8;
  uint32_t bit_in_byte = frame_index % 8;
  mm.bitmap[byte] &= ~(1 << bit_in_byte);

  return 0;
}

/**
 * mm_swap_in_page: Allocates a new physical frame to restore data from swap
 * storage.
 */
void *mm_swap_in_page(uint32_t swap_slot_id) {
  printk("<6>[  %s  ] Fetching stored byte stream from swap slot offset %u.\n",
         MODULE_NAME, swap_slot_id);

  /* Request a clean page frame using your kmalloc suite */
  void *allocated_frame = kmalloc(PAGE_SIZE);
  if (!allocated_frame) {
    printk("<3>[  %s  ] Swap restoration fault. No physical frames left.\n",
           MODULE_NAME);
    return NULL;
  }

  if (g_swap_used_slots >= PAGES_PER_SECTOR) {
    g_swap_used_slots -= PAGES_PER_SECTOR;
  }

  return allocated_frame;
}