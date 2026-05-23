#include <drivers/dax.h>
#include <mm/vmm.h>
#include <lib/string.h>

int dax_init(struct dax_device *dev, uintptr_t base, size_t size) {
    dev->phys_addr = base;
    dev->size = size;

    dev->virt_addr = vmm_map_io(base, size);

    if (!dev->virt_addr) return -1;
    return 0;
}


void* dax_direct_access(struct dax_device *dev, size_t offset) {
    if (offset >= dev->size) return NULL;

    return (void*)((uintptr_t)dev->virt_addr + offset);
}


int dax_copy_from_iter(struct dax_device *dev, size_t offset, void *src, size_t len) {
    void *dest = dax_direct_access(dev, offset);
    if (!dest) return -1;

    memcpy(dest, src, len);

#if defined(__x86__)
    asm volatile("clflush (%0)" : : "r"(dest));
#elif defined(__riscv)
    asm volatile("fence rw, io");
#endif

    return 0;
}
