#include <kernel/errno.h>
#include <kernel/printk.h>
#include <stddef.h>

#define MODULE_NAME "CLASS_SUBSYS"
#define MAX_CLASSES 32

typedef struct class {
  const char *name;
  void (*dev_release)(void *dev);
} class_t;

static const class_t *g_class_table[MAX_CLASSES];
static int g_class_count = 0;

/**
 * class_register: Mounts a high-level logical functional interface group type
 * indicator.
 */
int class_register(const class_t *cls) {
  if (!cls || !cls->name)
    return -EINVAL;
  if (g_class_count >= MAX_CLASSES)
    return -ENOMEM;

  g_class_table[g_class_count++] = cls;
  printk("<6>[  %s  ] Created logical driver interface class grouping: "
         "/sys/class/%s\n",
         MODULE_NAME, cls->name);

  return 0;
}

/**
 * class_init: Sets up structural base layouts.
 */
void class_init(void) {
  g_class_count = 0;
  printk("<6>[  %s  ] Device class subsystem abstraction registries "
         "initialized.\n",
         MODULE_NAME);
}