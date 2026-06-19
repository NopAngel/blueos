#include <kernel/colors.h>
#include <kernel/errno.h>
#include <kernel/printk.h>
#include <stddef.h>
#include <stdint.h>

#define MODULE_NAME "TTY_VCC"
#define MAX_VIRTUAL_CONSOLES 8

typedef struct {
  uint32_t console_id;
  int is_active;
  uint32_t cursor_x;
  uint32_t cursor_y;
  uint8_t current_color_attribute;
} virtual_console_t;

static virtual_console_t g_vcc_matrix[MAX_VIRTUAL_CONSOLES];
static uint32_t g_active_vcc_index = 0;

/**
 * vcc_switch_console: Changes the foreground active console layout routing.
 */
int vcc_switch_console(uint32_t target_id) {
  if (target_id >= MAX_VIRTUAL_CONSOLES)
    return -EINVAL;

  g_active_vcc_index = target_id;
  printk("<6>[  %s   ] Switching active text viewport display allocation to "
         "Console /dev/tty%u\n",
         MODULE_NAME, target_id);

  /* Trigger complete display redraw buffer refresh here */
  return 0;
}

/**
 * vcc_write: Routes text stream strings straight into the active screen buffer.
 */
int vcc_write(uint32_t console_id, const char *buf, size_t count) {
  if (console_id >= MAX_VIRTUAL_CONSOLES || !buf)
    return -EINVAL;

  /* Print only if it matches the current foreground terminal viewport */
  if (console_id == g_active_vcc_index) {
    // extern void video_raw_print(const char *str, size_t len);
    // video_raw_print(buf, count);
  }

  return (int)count;
}

/**
 * vcc_init: Initializes the Virtual Console Concentrator base arrays.
 */
void vcc_init(void) {
  boot_msg(MODULE_NAME,
           "Configuring Virtual Console Concentrator multiplexer lines...", 0);

  for (uint32_t i = 0; i < MAX_VIRTUAL_CONSOLES; i++) {
    g_vcc_matrix[i].console_id = i;
    g_vcc_matrix[i].is_active = 1;
    g_vcc_matrix[i].cursor_x = 0;
    g_vcc_matrix[i].cursor_y = 0;
  }

  g_active_vcc_index = 0; /* Default boot terminal */
  printk("<6>[  %s   ] Registered %u virtual terminal consoles instances "
         "(/dev/tty0 to /dev/tty7).\n",
         MODULE_NAME, MAX_VIRTUAL_CONSOLES);
}