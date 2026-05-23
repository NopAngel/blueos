#ifndef NET_H
#define NET_H

#include <stdint.h>

#define MAX_PACKET_SIZE 1514  // Ethernet (MTU 1500 + Header)

struct net_packet {
    uint8_t  data[MAX_PACKET_SIZE];
    uint32_t len;
    uint32_t flags;
    struct net_packet *next; 
};

int net_transmit(struct net_packet *pkt);
struct net_packet *net_alloc_packet();
void net_free_packet(struct net_packet *pkt);

#endif