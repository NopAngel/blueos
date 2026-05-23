/* BlueOS drivers/amba/amba_bus.h */
#include <stdint.h>

struct amba_device {
    uintptr_t res_start;   
    uintptr_t res_end;
    uint32_t periphid;   
    int irq;               
    void *dev_ptr;         
};

struct amba_driver {
    const char *name;
    uint32_t id_mask;
    int (*probe)(struct amba_device *dev);
    void (*remove)(struct amba_device *dev);
};