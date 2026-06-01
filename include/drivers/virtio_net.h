#ifndef _DRIVERS_VIRTIO_NET_H
#define _DRIVERS_VIRTIO_NET_H

#include <stdint.h>

typedef struct {
    uint8_t flags;
    uint8_t gso_type;
    uint16_t hdr_len;
    uint16_t gso_size;
    uint16_t csum_start;
    uint16_t csum_offset;
} __attribute__((packed)) virtio_net_hdr_t;

void virtio_net_init(uint32_t base);
void virtio_net_send_packet(void* packet, uint32_t len);
int virtio_net_poll_rx(void);

#endif
