#include <drivers/vhost.h>
#include <kernel/printk.h>
#include <kernel/colors.h>
#include <mm/memory.h> 

void vhost_init(void) {
    printk(GREEN, "BlueOS: Initializing VHOST Kernel Accelerator...\n");
}

void vhost_process_vq(struct vhost_virtqueue *vq) {
    uint16_t last_avail_idx = 0; 
    
    uint16_t avail_idx = *vq->avail;

    while (last_avail_idx != avail_idx) {
        struct vring_desc *desc = &vq->desc[last_avail_idx % vq->num];
    
        // process_packet((void*)desc->addr, desc->len);

        last_avail_idx++;
    }
}

int vhost_dev_open(struct vhost_dev *dev) {
    if (!dev) return -1;
    printk(WHITE, "VHOST: Device opened with %d queues\n", dev->nvqs);
    return 0;
}