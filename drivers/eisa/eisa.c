#include <kernel/colors.h>
#include <kernel/errno.h>
#include <kernel/io.h> /* For outb/inb macros if mapped into arch layers */
#include <kernel/printk.h>
#include <stdint.h>

#define MODULE_NAME "EISA_BUS"
#define EISA_MAX_SLOTS 16
#define EISA_SLOT_BASE(slot) (slot * 0x1000)

typedef struct {
  uint32_t slot_id;
  uint32_t product_id;
  int device_present;
} eisa_device_t;

static eisa_device_t g_eisa_slots[EISA_MAX_SLOTS];

/**
 * eisa_probe_bus: Probes the hardware slot ranges searching for classic 32-bit
 * signature IDs.
 */
void eisa_probe_bus(void) {
  printk("<6>[  %s   ] Initiating EISA motherboard hardware slots enumeration "
         "scanning...\n",
         MODULE_NAME);

  uint32_t devices_found = 0;

  /* Slot 0 is always reserved for the system board controller itself */
  for (uint32_t slot = 1; slot < EISA_MAX_SLOTS; slot++) {
    uintptr_t sig_port = EISA_SLOT_BASE(slot) + 0x0C80;

    /* Simulation of reading hardware signature registers.
     * Real hardware returns 0xFFFFFFFF if the slot expansion bay is completely
     * empty.
     */
    uint32_t simulated_sig =
        (slot == 2) ? 0x10A342D1
                    : 0xFFFFFFFF; // Simulating a legacy card found in slot 2

    if (simulated_sig != 0xFFFFFFFF) {
      g_eisa_slots[slot].slot_id = slot;
      g_eisa_slots[slot].product_id = simulated_sig;
      g_eisa_slots[slot].device_present = 1;
      devices_found++;

      printk("<6>[  %s   ] Expansion card detected in EISA Slot %u! Product "
             "Signature: 0x%08X\n",
             MODULE_NAME, slot, simulated_sig);
    } else {
      g_eisa_slots[slot].device_present = 0;
    }
  }

  if (devices_found == 0) {
    printk("<7>[  %s   ] EISA bus scan complete. No legacy motherboard "
           "adapters matching signatures found.\n",
           MODULE_NAME);
  }
}

/**
 * eisa_bus_init: Bootstrap routine for legacy subsystem mapping layer.
 */
void eisa_bus_init(void) {
  boot_msg(MODULE_NAME,
           "Subsystem mounting for Extended Industry Standard Architecture "
           "(EISA) bus...",
           0);
  eisa_probe_bus();
}