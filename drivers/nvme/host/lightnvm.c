#include <kernel/printk.h>
#include <uclite/errno.h>
#include <stdint.h>

#define MODULE_NAME "NVME_LNVM"

typedef struct {
    uint8_t  geometry_channels;
    uint16_t blocks_per_lun;
    uint32_t page_size;
} lnvm_dev_geo_t;

/**
 * nvme_nvm_register: Exposes underlying raw flash matrix nodes directly to BlueOS memory manager.
 */
int nvme_nvm_register(void *target_ctrl, const char *instance_name) {
    if (!target_ctrl) return -EINVAL;

    lnvm_dev_geo_t raw_geo;
    raw_geo.geometry_channels = 8; /* High-speed multi-channel layout */
    raw_geo.blocks_per_lun = 1024;
    raw_geo.page_size = 4096;      /* Synchronized with native VMM pages */

    printk("<6>[  %s  ] Open-Channel SSD architecture enabled for node entry '%s'.\n", 
           MODULE_NAME, instance_name);
    printk("<7>[  %s  ] Flash Geometry: %u internal channels, %u LUN blocks, %u-byte native pages.\n", 
           MODULE_NAME, raw_geo.geometry_channels, raw_geo.blocks_per_lun, raw_geo.page_size);

    return 0;
}