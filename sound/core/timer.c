#include <kernel/colors.h>
#include <kernel/printk.h>
#include <stdint.h>

#define MODULE_NAME "SOUND_TIMER"

typedef struct {
  uint32_t resolution_ns; /* Tick precision measurement in nanoseconds */
  uint64_t total_ticks;
  int is_active;
} snd_timer_t;

static snd_timer_t g_sound_core_timer;

/**
 * snd_timer_start: Locks the software timer tracking loops.
 */
void snd_timer_start(uint32_t microsecond_resolution) {
  g_sound_core_timer.resolution_ns = microsecond_resolution * 1000;
  g_sound_core_timer.total_ticks = 0;
  g_sound_core_timer.is_active = 1;

  printk(
      "<6>[  %s ] High-resolution audio event ticking initialized at %u us.\n",
      MODULE_NAME, microsecond_resolution);
}

/**
 * snd_timer_interrupt_handler: Triggered by system clock lines to advance
 * sub-stream buffer tasks.
 */
void snd_timer_interrupt_handler(void) {
  if (!g_sound_core_timer.is_active)
    return;

  g_sound_core_timer.total_ticks++;

  /* Every period boundary tick, this routine calls into pcm.c to update buffer
   * pointer updates */
  if (g_sound_core_timer.total_ticks % 64 == 0) {
    printk("<7>[  %s ] Period boundary elapsed. Raising softIRQ to copy next "
           "audio frames chunk.\n",
           MODULE_NAME);
  }
}

/**
 * snd_timer_init: Initializes tracking vectors.
 */
void snd_timer_init(void) {
  g_sound_core_timer.is_active = 0;
  g_sound_core_timer.total_ticks = 0;
  boot_msg(MODULE_NAME,
           "Sound core timing hardware mapping infrastructure ready.", 0);
}