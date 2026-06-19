#include <kernel/colors.h>
#include <kernel/errno.h>
#include <kernel/printk.h>
#include <stddef.h>
#include <stdint.h>

#define MODULE_NAME "DEVICE_CORE"
#define MAX_DEVICES 256

typedef struct device {
  const char *name;
  struct device *parent;
  void *driver_data;
  uint32_t device_id;
  int is_registered;
} device_t;

static device_t g_device_registry[MAX_DEVICES];
static uint32_t g_next_device_id = 0;

/**
 * device_register: Inserts a new physical hardware node into the core topology
 * tree.
 */
int device_register(device_t *dev) {
  if (!dev || !dev->name)
    return -EINVAL;
  if (g_next_device_id >= MAX_DEVICES) {
    printk(
        "<3>[  %s  ] Error: Global device registry tracking limit reached.\n",
        MODULE_NAME);
    return -ENOMEM;
  }

  dev->device_id = g_next_device_id++;
  dev->is_registered = 1;

  /* Copy handle reference into the static core registration matrix */
  extern void *mm_memcpy(void *dest, const void *src, size_t n);
  g_device_registry[dev->device_id] = *dev;

  if (dev->parent) {
    printk("<6>[  %s  ] Registered device '%s' linked under parent node '%s' "
           "(ID: %u)\n",
           MODULE_NAME, dev->name, dev->parent->name, dev->device_id);
  } else {
    printk("<6>[  %s  ] Registered root host device '%s' (ID: %u)\n",
           MODULE_NAME, dev->name, dev->device_id);
  }

  return 0;
}

/**
 * drivers_base_init: Framework bootstrap architecture layout mapping.
 */
void drivers_base_init(void) {
  boot_msg(MODULE_NAME,
           "Initializing Core Object Infrastructure unified device model...",
           0);
  for (int i = 0; i < MAX_DEVICES; i++) {
    g_device_registry[i].is_registered = 0;
  }
}