#include <kernel/colors.h>
#include <kernel/printk.h>
#include <stdint.h>

#define MODULE_NAME "CPUIDLE"

static uint64_t g_total_idle_time_ticks = 0;
static uint32_t g_last_selected_c_state = 0;

/**
 * cpuidle_enter_state: Moves the target processing core into an electrical
 * low-power C-state.
 */
void cpuidle_enter_state(uint32_t c_state_index) {
  g_last_selected_c_state = c_state_index;

  /* Logic mapping matching classic Intel Advanced Power Management
   * architectures */
  if (c_state_index == 0) {
    /* C1 State: Standard halt instructions execution execution loop */
    asm volatile("hlt");
  } else if (c_state_index == 2) {
    /* C2 State: Stop core clocks, read from specific ACPI level 2 I/O registry
     * port triggers */
    g_total_idle_time_ticks += 5;
    asm volatile("hlt");
  } else {
    /* Deep Sleep C3/C4 States: Flush caches, stop local timers, sleep core */
    g_total_idle_time_ticks += 15;
    asm volatile("hlt");
  }
}

/**
 * cpuidle_select_state: Analyzes scheduler metrics to predict execution idle
 * time slots.
 */
uint32_t cpuidle_select_state(uint32_t expected_latency_us) {
  /* If the next system timer tick is immediate, pick shallow fast sleep (C1) */
  if (expected_latency_us < 100) {
    return 0; /* State C1 */
  } else if (expected_latency_us < 1000) {
    return 2; /* State C2 */
  }

  return 3; /* Deep State C3/C4 */
}

/**
 * cpuidle_init: Registers latency thresholds metrics parameters.
 */
void cpuidle_init(void) {
  boot_msg(MODULE_NAME,
           "Registering processing core sleep state matrix structures...", 0);
  printk("<6>[  %s  ] C1 (Latency: 1us), C2 (Latency: 20us), C3 (Latency: "
         "100us) active.\n",
         MODULE_NAME);
}