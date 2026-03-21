#ifndef CXL_H
#define CXL_H

#include <stdint.h>

/* CXL Capability IDs */
#define CXL_DVSEC_VENDOR_ID        0x1E98
#define CXL_DVSEC_FLEX_BUS_REGS    0x0
#define CXL_DVSEC_MEM_DEVICE       0x3

/* CXL Register Offsets (Component Registers) */
#define CXL_RAS_CAP_ID             0x00
#define CXL_HDM_DECODER_CAPABILITY 0x10
#define CXL_HDM_DECODER0_BASE_LOW  0x20
#define CXL_HDM_DECODER0_SIZE_LOW  0x28

struct cxl_device {
    uint16_t pci_dev;
    uintptr_t component_regs; 
    uint64_t mem_size;        
};

int cxl_init();
int cxl_enumerate_devices();
void cxl_map_memory(struct cxl_device *dev);

#endif