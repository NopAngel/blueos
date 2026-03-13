#include <stdint.h>
#include <blueos/printk.h>

// NOOP NOOP
void i386_init_noop(void) { 
    // LOL
}

struct i386_init_ops {
    void (*timer_init)(void);
    void (*rtc_init)(void);
    void (*keyboard_init)(void);
    void (*pci_init)(void);
};

struct i386_init_ops blueos_init = {
    .timer_init    = i386_init_noop,
    .rtc_init      = i386_init_noop,
    .keyboard_init = i386_init_noop,
    .pci_init      = i386_init_noop
};