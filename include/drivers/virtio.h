#ifndef _DRIVERS_VIRTIO_H
#define _DRIVERS_VIRTIO_H

#include <stdint.h>

#define VIRTIO_PCI_DEVICE_ID        0x1003 // Console
#define VIRTIO_CONSOLE_VQS          2      // ReceiveQ and TransmitQ

typedef struct {
    uint32_t addr;
    uint32_t len;
    uint16_t flags;     // VRING_DESC_F_NEXT, etc.
    uint16_t next;
} __attribute__((packed)) vring_desc_t;

typedef struct {
    uint16_t flags;
    uint16_t idx;
    uint16_t ring[0];
} __attribute__((packed)) vring_avail_t;

typedef struct {
    uint16_t flags;
    uint16_t idx;
    struct { uint32_t id; uint32_t len; } ring[0];
} __attribute__((packed)) vring_used_t;


void virtio_console_init(uint32_t pci_base);

void virtio_console_send(const char* buf, uint32_t len);

#endif
