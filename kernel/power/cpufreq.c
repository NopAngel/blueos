#include <kernel/colors.h>
#include <kernel/errno.h>
#include <kernel/printk.h>
#include <stddef.h>
#include <stdint.h>

#define MODULE_NAME "CPUFREQ"

typedef struct {
  char name[16];
  uint32_t min_freq_mhz;
  uint32_t max_freq_mhz;
  uint32_t current_freq_mhz;
} cpufreq_policy_t;

static cpufreq_policy_t g_cpu_policy;

/**
 * cpufreq_set_governor: Changes the operational scaling rules of the processor
 * core.
 */
int cpufreq_set_governor(const char *governor_mode) {
  if (!governor_mode)
    return -EFAULT;

  extern int mm_memset(void *s, int c, size_t n);
  extern void *mm_memcpy(void *dest, const void *src, size_t n);

  if (governor_mode[0] == 'p') { /* "performance" tracking fallback rule */
    g_cpu_policy.current_freq_mhz = g_cpu_policy.max_freq_mhz;
    printk("<6>[  %s  ] Policy updated -> 'Performance'. Frequency locked at: "
           "%d MHz\n",
           MODULE_NAME, g_cpu_policy.current_freq_mhz);
  } else if (governor_mode[0] == 'p' &&
             governor_mode[1] == 'o') { /* "powersave" rule */
    g_cpu_policy.current_freq_mhz = g_cpu_policy.min_freq_mhz;
    printk("<6>[  %s  ] Policy updated -> 'Powersave'. Frequency scaled down "
           "to: %d MHz\n",
           MODULE_NAME, g_cpu_policy.current_freq_mhz);
  } else {
    /* Default generic on-demand allocation configuration setup handling */
    g_cpu_policy.current_freq_mhz =
        (g_cpu_policy.max_freq_mhz + g_cpu_policy.min_freq_mhz) / 2;
    printk("<6>[  %s  ] Policy updated -> 'Ondemand'. Dynamic scaling core "
           "active at: %d MHz\n",
           MODULE_NAME, g_cpu_policy.current_freq_mhz);
  }

  /* Interface drivers execute architecture-specific updates to MSR 0x199
   * (IA32_PERF_CTL) here */
  return 0;
}

/**
 * cpufreq_init: Queries CPU capabilities and initializes frequency policies.
 */
void cpufreq_init(void) {
  boot_msg(MODULE_NAME, "Initializing CPU Frequency scaling subsystem...", 0);

  /* Hardware probe simulation values: 2.0 GHz Base, 3.5 GHz Turbo Boost
   * capability */
  g_cpu_policy.min_freq_mhz = 2000;
  g_cpu_policy.max_freq_mhz = 3500;

  cpufreq_set_governor("ondemand");
}