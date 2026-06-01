#include <kernel/net.h>
#include <drivers/virtio_net.h>
#include <kernel/printk.h>

/**
 * Calcula el checksum RFC 1071 (Internet Checksum)
 * Necesario para que el paquete no sea rechazado por el router.
 */
uint16_t calculate_checksum(void* addr, int len) {
    uint16_t* ptr = (uint16_t*)addr;
    uint32_t sum = 0;
    while (len > 1) {
        sum += *ptr++;
        len -= 2;
    }
    if (len > 0) {
        sum += *(uint8_t*)ptr;
    }
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    return (uint16_t)~sum;
}

/**
 * Envía un ping ICMP real a una IP destino.
 * @param target_ip: IP destino en formato Little Endian (ej: 0x0202000A para 10.0.2.2)
 */
void send_raw_ping(uint32_t target_ip) {
    uint8_t frame[64] = {0}; // Frame Ethernet mínimo (64 bytes)

    ipv4_packet_t* ip = (ipv4_packet_t*)(frame + 14);
    icmp_packet_t* icmp = (icmp_packet_t*)(frame + 34);

    ip->ver_ihl = 0x45;
    ip->tos = 0;
    ip->total_len = 52;       // 20 (IP) + 32 (ICMP) = 52 bytes
    ip->id = 0x1234;
    ip->flags_frag = 0;
    ip->ttl = 64;
    ip->proto = 1;
    ip->checksum = 0;
    ip->src_ip = 0x0F02000A;
    ip->dst_ip = target_ip;

    icmp->type = 8;           // Echo Request
    icmp->code = 0;
    icmp->checksum = 0;
    icmp->id = 0xABCD;
    icmp->seq = 1;
    for(int i = 0; i < 32; i++) icmp->payload[i] = 'B'; // "B" BlueOS!

    icmp->checksum = calculate_checksum(icmp, 32);

    virtio_net_send_packet(frame, 64);

    printk("\033[36m[NET]\033[0m Ping enviado a %x\n", target_ip);
}
