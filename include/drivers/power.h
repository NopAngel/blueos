#ifndef POWER_H
#define POWER_H

#include <stdint.h>

typedef enum {
    SYS_POWER_ON,
    SYS_POWER_SLEEP,
    SYS_POWER_REBOOT,
    SYS_POWER_OFF
} power_state_t;

void sys_reboot(void);
void sys_shutdown(void);
void sys_set_state(power_state_t state);

// Para el banner de BlueOS
int sys_get_battery_level(void);

#endif
