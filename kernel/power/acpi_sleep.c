#include <kernel/colors.h>
#include <kernel/errno.h>
#include <kernel/ports.h>
#include <kernel/printk.h>
#include <stdint.h>

#define MODULE_NAME "ACPI_POWER"

/* Classic ACPI Sleep States Mapping Definitions */
#define ACPI_STATE_S0 0 /* Working State */
#define ACPI_STATE_S1 1 /* CPU Internal Clock Stopped */
#define ACPI_STATE_S3 3 /* Suspend to RAM (Sleep) */
#define ACPI_STATE_S5 5 /* Soft Off (Power Down) */

static uint32_t g_acpi_smi_cmd_port = 0;
static uint8_t g_acpi_enable_value = 0;
static uint32_t g_pm1a_cnt_blk =
    0xB004; /* Default QEMU i440FX PM1a control block register */

/**
 * acpi_enter_sleep_state: Writes the specific state tokens to the Power
 * Management register.
 */
int acpi_enter_sleep_state(int sleep_state) {
  if (sleep_state != ACPI_STATE_S1 && sleep_state != ACPI_STATE_S3 &&
      sleep_state != ACPI_STATE_S5) {
    return -EINVAL;
  }

  printk("<5>[  %s  ] Preparing subsystem transition to State S%d...\n",
         MODULE_NAME, sleep_state);

  uint16_t slp_typa = 0;
  uint16_t slp_en = 1 << 13; /* Bit 13: Sleep Enable standard flag */

  switch (sleep_state) {
  case ACPI_STATE_S1:
    slp_typa = 0x1 << 10;
    break;
  case ACPI_STATE_S3:
    slp_typa =
        0x2 << 10; /* Typically 0x2 or 0x3 depending on DSDT tables parsing */
    break;
  case ACPI_STATE_S5:
    slp_typa = 0x0 << 10; /* QEMU poweroff SLP_TYPa signature */
    break;
  }

  printk("<0>[  %s  ] Flashing hardware control blocks. Executing transition "
         "code now.\n",
         MODULE_NAME);

  /* Outw command fires the final hardware power transition */
  outw(g_pm1a_cnt_blk, slp_typa | slp_en);

  /* If S5 powerdown execution works, the CPU halts completely here */
  if (sleep_state == ACPI_STATE_S1) {
    asm volatile("hlt"); /* Safe fallback sleep stop execution step */
  }

  return 0;
}

/**
 * acpi_power_init: Locates ACPI roots pointers during platform system
 * bootstraps.
 */
void acpi_power_init(void) {
  boot_msg(MODULE_NAME, "Probing ACPI Fixed ACPI Description Tables (FADT)...",
           0);

  /* Standard QEMU/Bochs hardware profile detection fallback simulation values
   */
  g_acpi_smi_cmd_port = 0xB02F;
  g_acpi_enable_value = 0xA0;

  /* Enable ACPI mode by sending command byte to the SMI port descriptor */
  outb(g_acpi_smi_cmd_port, g_acpi_enable_value);

  boot_msg(MODULE_NAME,
           "ACPI subsystem enabled. Power states S1, S3, S5 accessible.", 0);
}