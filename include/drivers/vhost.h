#ifndef _BLUEOS_VHOST_H
#define _BLUEOS_VHOST_H

#include <stdint.h>
#include <stddef.h>

struct vring_desc {
    uint64_t addr;
    uint32_t len;
    uint16_t flags;
    uint16_t next;
};

struct vhost_virtqueue {
    uint32_t num;
    struct vring_desc *desc;
    uint16_t *avail;
    uint16_t *used;
    int kick_fd;   
    int call_fd; 
};

struct vhost_dev {
    struct vhost_virtqueue *vqs;
    int nvqs;
    uint64_t features;
};

void vhost_init(void);
int vhost_dev_open(struct vhost_dev *dev);
#endif