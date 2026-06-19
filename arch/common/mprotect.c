#include <kernel/errno.h>
#include <kernel/printk.h>
#include <mm/memory.h>
#include <stddef.h>
#include <stdint.h>

#define MODULE_NAME "MPROTECT"

/* Classic POSIX abstraction flags for page properties */
#define PROT_NONE 0x00  /* Page cannot be accessed */
#define PROT_READ 0x01  /* Page can be read */
#define PROT_WRITE 0x02 /* Page can be written */
#define PROT_EXEC 0x04  /* Page can be executed */

/**
 * mm_mprotect_set_flags: Modifies runtime operational constraints on physical
 * pages. Integrates directly with BlueOS bit manipulation masks.
 */
int mm_mprotect_set_flags(uintptr_t address, size_t length, int prot_flags) {
  if (address < mm.phys_limit_start || length == 0)
    return -EINVAL;

  uint32_t start_frame = (address - mm.phys_limit_start) / PAGE_SIZE;
  uint32_t end_frame =
      ((address + length + PAGE_SIZE - 1) - mm.phys_limit_start) / PAGE_SIZE;

  printk("<5>[  %s  ] Reconfiguring memory boundaries from frame %u to %u.\n",
         MODULE_NAME, start_frame, end_frame);

  for (uint32_t i = start_frame; i < end_frame; i++) {
    if (i >= mm.total_pages)
      return -ENOMEM;

    /* If the page is strictly protected by the core kernel initialization,
     * refuse overrides */
    if (mm_is_page_protected(i)) {
      printk("<4>[  %s  ] Access denied. Page index %u is locked by KASLR/Core "
             "policy.\n",
             MODULE_NAME, i);
      return -EACCES;
    }

    /* Dynamically managing protection masks based on requested flags */
    uint32_t byte = i / 8;
    uint32_t bit_in_byte = i % 8;

    if (prot_flags == PROT_NONE) {
      /* Restrict allocations: mark it as blocked inside the universal
       * protection tracker */
      mm.protection_bitmap[byte] |= (1 << bit_in_byte);
    } else {
      /* Enable availability: lift blocks if acceptable read/write privileges
       * are present */
      mm.protection_bitmap[byte] &= ~(1 << bit_in_byte);
    }
  }

  printk("<6>[  %s  ] Memory permission context updated successfully. Flag "
         "mask applied: 0x%X\n",
         MODULE_NAME, prot_flags);
  return 0;
}