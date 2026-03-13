// include/net/network.h o ponlo arriba de rtl8139.c para probar
#ifndef NETWORK_H
#define NETWORK_H

#include <stdint.h>

struct eth_header {
    uint8_t dest[6];
    uint8_t src[6];
    uint16_t type;
} __attribute__((packed));

struct ip_header {
    uint8_t ver_ihl;
    uint8_t tos;
    uint16_t len;
    uint16_t id;
    uint16_t flags_offset;
    uint8_t ttl;
    uint8_t proto;
    uint16_t checksum;
    uint32_t src_ip;
    uint32_t dest_ip;
} __attribute__((packed));

struct icmp_header {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint16_t id;
    uint16_t seq;
} __attribute__((packed));

struct tcp_header {
    uint16_t src_port;
    uint16_t dest_port;
    uint32_t seq_num;
    uint32_t ack_num;
    uint8_t  data_offset; // Offset y Reservado
    uint8_t  flags;       // FIN, SYN, RST, PSH, ACK, URG
    uint16_t window_size;
    uint16_t checksum;
    uint16_t urgent_ptr;
} __attribute__((packed));

struct tcp_pseudo_header {
    uint32_t src_ip;
    uint32_t dest_ip;
    uint8_t  zeros;
    uint8_t  proto;
    uint16_t tcp_len;
} __attribute__((packed));

struct arp_packet {
    uint16_t hw_type;      // Ethernet = 1
    uint16_t proto_type;   // IPv4 = 0x0800
    uint8_t  hw_len;       // MAC = 6
    uint8_t  proto_len;    // IP = 4
    uint16_t opcode;       // 1 = Request, 2 = Reply
    uint8_t  src_mac[6];
    uint32_t src_ip;
    uint8_t  dest_mac[6];
    uint32_t dest_ip;
} __attribute__((packed));

// Prototipos para que no den "implicit declaration"
uint16_t htons(uint16_t hostshort);
uint32_t htonl(uint32_t hostlong);
uint16_t net_checksum(void *vdata, uint32_t length);

#endif

