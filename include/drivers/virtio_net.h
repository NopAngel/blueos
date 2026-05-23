#ifndef VIRTIO_NET_H
#define VIRTIO_NET_H

#include <stdint.h>

struct virtio_net_config {
    uint8_t  mac[6];
    uint16_t status;
    uint16_t max_virtqueue_pairs;
    uint16_t mtu;
} __attribute__((packed)); 

#endif