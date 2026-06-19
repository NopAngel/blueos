#include <kernel/colors.h>
#include <kernel/errno.h>
#include <kernel/printk.h>
#include <stddef.h>
#include <stdint.h>

#define MODULE_NAME "EXTCON_CORE"
#define MAX_EXTCON_DEVICES 8

/* Supported external connector type definitions mapping */
#define EXTCON_USB 1
#define EXTCON_JACK_AUDIO 2
#define EXTCON_HDMI 3

typedef struct {
  const char *name;
  uint32_t supported_cables;
  uint32_t state; /* Bitmask of currently attached cables */
  int is_registered;
} extcon_dev_t;

static extcon_dev_t g_extcon_registry[MAX_EXTCON_DEVICES];
static uint32_t g_extcon_count = 0;

/**
 * extcon_set_state: Updates the connection bitmask state and broadcasts
 * notifications.
 */
int extcon_set_state(uint32_t index, uint32_t cable_type, int is_attached) {
  if (index >= g_extcon_count || !g_extcon_registry[index].is_registered) {
    return -EINVAL;
  }

  extcon_dev_t *edev = &g_extcon_registry[index];

  if (is_attached) {
    edev->state |= (1 << cable_type);
    printk("<6>[  %s   ] Cable state connected on '%s'. Type token ID: %u\n",
           MODULE_NAME, edev->name, cable_type);
  } else {
    edev->state &= ~(1 << cable_type);
    printk("<6>[  %s   ] Cable state disconnected on '%s'. Type token ID: %u\n",
           MODULE_NAME, edev->name, cable_type);
  }

  /* Trigger underlying jump_labels or notifier chain vectors here */
  return 0;
}

/**
 * extcon_register_device: Adds an external connector device subsystem node
 * topology.
 */
int extcon_register_device(const char *name, uint32_t cable_mask) {
  if (g_extcon_count >= MAX_EXTCON_DEVICES)
    return -ENOMEM;

  uint32_t slot = g_extcon_count++;
  g_extcon_registry[slot].name = name;
  g_extcon_registry[slot].supported_cables = cable_mask;
  g_extcon_registry[slot].state = 0;
  g_extcon_registry[slot].is_registered = 1;

  printk(
      "<6>[  %s   ] Registered external connector channel class device: '%s'\n",
      MODULE_NAME, name);
  return (int)slot;
}

/**
 * extcon_init: Subsystem bootstrap launcher entry.
 */
void extcon_init(void) {
  g_extcon_count = 0;
  boot_msg(
      MODULE_NAME,
      "Initializing External Connector Subsystem framework abstractions...", 0);
}