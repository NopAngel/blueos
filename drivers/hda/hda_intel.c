#include <kernel/colors.h>
#include <kernel/printk.h>
#include <kernel/timer.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#define MODULE_NAME "HDA_INTEL"

/* Intel HD Audio PCI Configuration Registers (MMIO Base Offsets) */
#define HDA_REG_GCAP 0x00      /* Global Capabilities */
#define HDA_REG_GCTL 0x08      /* Global Control */
#define HDA_REG_WAKEEN 0x0C    /* Wake Enable */
#define HDA_REG_INTCTL 0x20    /* Interrupt Control */
#define HDA_REG_INTSTS 0x24    /* Interrupt Status */
#define HDA_REG_CORBLBASE 0x40 /* CORB Lower Base Address */
#define HDA_REG_CORBUBASE 0x44 /* CORB Upper Base Address */
#define HDA_REG_RIRBLBASE 0x50 /* RIRB Lower Base Address */

/* Struct representing the physical state of the hardware controller */
typedef struct {
  uintptr_t bar0_address;
  uint16_t pci_vendor_id;
  uint16_t pci_device_id;
  int total_streams;
  int irq_line;
  int is_initialized;
} hda_controller_t;

static hda_controller_t g_hda_card = {0};

/* External driver helper defined inside hda_codec.c */
extern void hda_codec_probe_bus(uintptr_t base_addr);

/**
 * hda_intel_read32: Helper routine to read from MMIO space
 */
static inline uint32_t hda_intel_read32(uint32_t offset) {
  return *(volatile uint32_t *)(g_hda_card.bar0_address + offset);
}

/**
 * hda_intel_write32: Helper routine to write into MMIO space
 */
static inline void hda_intel_write32(uint32_t offset, uint32_t value) {
  *(volatile uint32_t *)(g_hda_card.bar0_address + offset) = value;
}

/**
 * hda_intel_reset: Forces the Intel HDA controller back to a pristine state
 */
int hda_intel_reset(void) {
  uint32_t gctl = hda_intel_read32(HDA_REG_GCTL);

  /* Clear CRST bit to reset the controller hardware pipelines */
  hda_intel_write32(HDA_REG_GCTL, gctl & ~(1 << 0));

  /* Wait for the hardware to settle loop */
  for (volatile int i = 0; i < 10000; i++)
    ;

  /* Bring the hardware out of reset */
  hda_intel_write32(HDA_REG_GCTL, hda_intel_read32(HDA_REG_GCTL) | (1 << 0));

  /* Await hardware initialization response */
  int timeout = 1000;
  while (!(hda_intel_read32(HDA_REG_GCTL) & (1 << 0)) && --timeout)
    ;

  return (timeout > 0) ? 0 : -1;
}

/**
 * hda_intel_init: Master PCI architecture entry point for the sound card setup
 */
int hda_intel_init(uintptr_t mmio_base, int irq) {
  g_hda_card.bar0_address = mmio_base;
  g_hda_card.irq_line = irq;
  g_hda_card.pci_vendor_id = 0x8086; /* Intel Vendor Signatures */
  g_hda_card.pci_device_id = 0x27D8; /* High Definition Audio Controller ID */

  boot_msg(MODULE_NAME, "Probing Intel HD Audio Hardware Engine...", 0);
  printk("<6>[  HDA_INTEL  ] MMIO Register space mapped at: %p\n",
         (void *)mmio_base);

  /* Execute a pristine cold hardware reset */
  if (hda_intel_reset() < 0) {
    boot_msg(MODULE_NAME, "Controller reset routine timed out!", 2);
    return -1;
  }

  /* Read Global Capabilities Register to parse stream info */
  uint16_t gcap = hda_intel_read32(HDA_REG_GCAP) & 0xFFFF;
  g_hda_card.total_streams = ((gcap >> 8) & 0x0F) + ((gcap >> 12) & 0x0F);

  printk("<6>[  HDA_INTEL  ] Hardware limits: %d DMA pipelines detected.\n",
         g_hda_card.total_streams);

  /* Turn on Master Interrupt Controls */
  hda_intel_write32(HDA_REG_INTCTL, (1 << 31) | (1 << 30));
  boot_msg(MODULE_NAME, "Interrupt handlers attached to audio vector loop.", 0);

  g_hda_card.is_initialized = 1;

  /* Hand off execution to the codec bus initialization interface */
  hda_codec_probe_bus(g_hda_card.bar0_address);

  return 0;
}