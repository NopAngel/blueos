#ifndef DAX_H
#define DAX_H

#include <stdint.h>
#include <stddef.h>

struct dax_device {
    uintptr_t phys_addr;  
    size_t size;          
    void* virt_addr;     
};

int dax_init(struct dax_device *dev, uintptr_t base, size_t size);
void* dax_direct_access(struct dax_device *dev, size_t offset);
int dax_copy_from_iter(struct dax_device *dev, size_t offset, void *src, size_t len);

#endif