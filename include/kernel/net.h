#ifndef _KERNEL_NET_H
#define _KERNEL_NET_H

#include <stdint.h>

typedef struct {
    uint8_t ver_ihl;
    uint8_t tos;
    uint16_t total_len;
    uint16_t id;
    uint16_t flags_frag;
    uint8_t ttl;
    uint8_t proto;
    uint16_t checksum;
    uint32_t src_ip;
    uint32_t dst_ip;
} __attribute__((packed)) ipv4_packet_t;

typedef struct {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint16_t id;
    uint16_t seq;
    char payload[32];
} __attribute__((packed)) icmp_packet_t;

uint16_t calculate_checksum(void* addr, int len);

#endif
