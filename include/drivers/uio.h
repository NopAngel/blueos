#ifndef UIO_H
#define UIO_H

#include <stdint.h>
#include <stddef.h>

struct uio_info {
    const char *name;
    uintptr_t addr;    
    size_t size;         
    int irq;             
    int uio_id;         
};

int uio_register_device(struct uio_info *info);
void uio_unregister_device(struct uio_info *info);
void uio_interrupt_handler(int irq);

#endif