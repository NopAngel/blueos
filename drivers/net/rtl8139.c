#include <net/network.h>
#include <kernel/ports.h>
#include <kernel/printk.h>
#include <kernel/colors.h>
#include <mm/memory.h>

#include <lib/string.h>


static uint32_t io_base;
static uint8_t mac[6];
static uint32_t rx_ptr = 0;

uint8_t gateway_mac[6] = {0, 0, 0, 0, 0, 0};
uint32_t tcp_seq = 0x12345678;
uint32_t tcp_ack = 0;

static uint8_t rx_buffer[8192 + 16 + 1500] __attribute__((aligned(4096)));
uint16_t htons(uint16_t v) {
    return (uint16_t)((v << 8) | (v >> 8));
}

uint16_t ntohs(uint16_t v) {
    return htons(v);
}

uint32_t htonl(uint32_t v) {
    return ((v & 0xFF) << 24) | ((v & 0xFF00) << 8) | ((v & 0xFF0000) >> 8) | ((v & 0xFF000000) >> 24);
}

uint32_t ntohl(uint32_t v) {
    return htonl(v);
}






uint16_t net_checksum(void *vdata, uint32_t length)  {
    uint8_t *data = vdata;
    uint32_t acc = 0xffff;
    for (size_t i = 0; i + 1 < length; i += 2) {
        uint16_t word;
        mm_memcpy(&word, data + i, 2);
        acc += htons(word);
        if (acc > 0xffff) acc -= 0xffff;
    }
    if (length & 1) {
        uint16_t word = 0;
        mm_memcpy(&word, data + length - 1, 1);
        acc += htons(word);
        if (acc > 0xffff) acc -= 0xffff;
    }
    return ntohs(~acc);
}

/* --- TRANSMISIÓN DE PAQUETES --- */

void send_arp_reply(uint8_t *target_mac, uint32_t target_ip) {
    uint8_t packet[64];
    mm_memset(packet, 0, 64);
    struct eth_header *eth = (struct eth_header *)packet;
    struct arp_packet *arp = (struct arp_packet *)(packet + sizeof(struct eth_header));

    for(int i=0; i<6; i++) eth->dest[i] = target_mac[i];
    for(int i=0; i<6; i++) eth->src[i] = mac[i];
    eth->type = htons(0x0806);

    arp->hw_type = htons(1);
    arp->proto_type = htons(0x0800);
    arp->hw_len = 6;
    arp->proto_len = 4;
    arp->opcode = htons(2); // REPLY

    for(int i=0; i<6; i++) arp->src_mac[i] = mac[i];
    uint8_t *s = (uint8_t*)&arp->src_ip;
    s[0] = 10; s[1] = 0; s[2] = 2; s[3] = 15;

    for(int i=0; i<6; i++) arp->dest_mac[i] = target_mac[i];
    arp->dest_ip = target_ip;

    outl(io_base + 0x20, (uint32_t)packet);
    outl(io_base + 0x10, 64);
    printk(BLUE, "[ ARP ] Sent Reply to Gateway\n");
}

void send_tcp_ack(uint32_t dest_ip, uint16_t dest_port, uint16_t src_port) {
    uint8_t packet[74];
    mm_memset(packet, 0, 74);
    struct eth_header *eth = (struct eth_header *)packet;
    struct ip_header *ip = (struct ip_header *)(packet + sizeof(struct eth_header));
    struct tcp_header *tcp = (struct tcp_header *)(packet + sizeof(struct eth_header) + sizeof(struct ip_header));

    for(int i=0; i<6; i++) eth->dest[i] = gateway_mac[i];
    for(int i=0; i<6; i++) eth->src[i] = mac[i];
    eth->type = htons(0x0800);

    ip->ver_ihl = 0x45;
    ip->proto = 6;
    ip->len = htons(40);
    ip->ttl = 64;
    uint8_t *s = (uint8_t*)&ip->src_ip;
    s[0] = 10; s[1] = 0; s[2] = 2; s[3] = 15;
    ip->dest_ip = dest_ip;
    ip->checksum = net_checksum(ip, 20);

    tcp->src_port = src_port;
    tcp->dest_port = dest_port;
    tcp->seq_num = htonl(tcp_seq);
    tcp->ack_num = htonl(tcp_ack);
    tcp->data_offset = 0x50;
    tcp->flags = 0x10; // ACK
    tcp->window_size = htons(64240);

    outl(io_base + 0x20, (uint32_t)packet);
    outl(io_base + 0x10, 74);
}

void send_http_get(uint32_t dest_ip, uint16_t dest_port, uint16_t src_port) {
    uint8_t packet[256];
    mm_memset(packet, 0, 256);
    char *http_request = "GET / HTTP/1.1\r\nHost: 93.184.216.34\r\nConnection: close\r\n\r\n";
    int http_len = strlen(http_request);

    struct eth_header *eth = (struct eth_header *)packet;
    struct ip_header *ip = (struct ip_header *)(packet + sizeof(struct eth_header));
    struct tcp_header *tcp = (struct tcp_header *)(packet + sizeof(struct eth_header) + sizeof(struct ip_header));
    uint8_t *payload = (uint8_t *)(packet + 54);

    for(int i=0; i<6; i++) eth->dest[i] = gateway_mac[i];
    for(int i=0; i<6; i++) eth->src[i] = mac[i];
    eth->type = htons(0x0800);

    ip->ver_ihl = 0x45;
    ip->proto = 6;
    ip->len = htons(40 + http_len);
    ip->ttl = 64;
    uint8_t *sip = (uint8_t*)&ip->src_ip;
    sip[0] = 10; sip[1] = 0; sip[2] = 2; sip[3] = 15;
    ip->dest_ip = dest_ip;
    ip->checksum = net_checksum(ip, 20);

    tcp->src_port = src_port;
    tcp->dest_port = dest_port;
    tcp->seq_num = htonl(tcp_seq);
    tcp->ack_num = htonl(tcp_ack);
    tcp->data_offset = 0x50;
    tcp->flags = 0x18; // PSH + ACK
    tcp->window_size = htons(64240);

    mm_memcpy(payload, http_request, http_len);

    outl(io_base + 0x20, (uint32_t)packet);
    outl(io_base + 0x10, 54 + http_len);
    printk(CYAN, "[ HTTP ] GET Request Sent!\n");
}

/**
 * RTL8139_HANDLER - El motor de red corregido para BlueOS
 */
void rtl8139_handler() {
    uint16_t status = inw(io_base + 0x3E);
    outw(io_base + 0x3E, status);

    if (status & 0x01) {
        while(!(inb(io_base + 0x37) & 0x01)) {

            uint16_t *packet_header = (uint16_t*)(rx_buffer + rx_ptr);
            uint16_t packet_status = packet_header[0];
            uint16_t packet_len    = packet_header[1];

            uint8_t *data = (uint8_t*)(rx_buffer + rx_ptr + 4);

            struct eth_header *eth = (struct eth_header *)data;
            uint16_t eth_type = htons(eth->type);

            if (eth_type == 0x0806) { // ARP
                struct arp_packet *arp = (struct arp_packet *)(data + 14);
                if (htons(arp->opcode) == 2) { // Reply
                    for(int i=0; i<6; i++) gateway_mac[i] = arp->src_mac[i];
                    printk(GREEN, "[ ARP ] Gateway MAC resolved!\n");
                }
            }
            else if (eth_type == 0x0800) { // IPv4
                struct ip_header *ip = (struct ip_header *)(data + 14);

                if (ip->proto == 1) { // ICMP (Ping)
                    printk(GREEN, "[ NET ] PONG received!\n");
                }
                else if (ip->proto == 6) { // TCP (Curl)
                    struct tcp_header *tcp = (struct tcp_header *)(data + 34);


                    if (tcp->flags == 0x12) { // SYN-ACK
                        tcp_ack = htonl(tcp->seq_num) + 1;
                        tcp_seq = htonl(tcp->ack_num);

                        send_tcp_ack(ip->src_ip, tcp->src_port, tcp->dest_port);
                        send_http_get(ip->src_ip, tcp->src_port, tcp->dest_port);
                    }
                    else if ((tcp->flags & 0x08) || packet_len > 60) {
                        uint8_t *html = (uint8_t *)(data + 54);
                        int h_len = packet_len - 54 - 4; // -4

                        if (h_len > 0) {
                            printk(WHITE, "\n--- BLUEOS WEB CONTENT ---\n");
                            for(int i=0; i < h_len; i++) {
                                if(html[i] >= 32 && html[i] <= 126) printk(WHITE, "%c", html[i]);
                                else if(html[i] == '\n') printk(WHITE, "\n");
                            }
                            printk(WHITE, "\n--------------------------\n");
                        }
                    }
                }
            }

            rx_ptr = (rx_ptr + packet_len + 4 + 3) & ~3;

            if (rx_ptr >= 8192) {
                rx_ptr -= 8192;
            }

            outw(io_base + 0x38, rx_ptr - 16);
        }
    }
}


void rtl8139_init(uint8_t bus, uint8_t slot) {
    io_base = pci_read_config(bus, slot, 0, 0x10) & ~0x3;
    uint32_t pci_command = pci_read_config(bus, slot, 0, 0x04);
    pci_write_config(bus, slot, 0, 0x04, pci_command | 0x4);

    outb(io_base + 0x52, 0x00);
    outb(io_base + 0x37, 0x10);
    while((inb(io_base + 0x37) & 0x10) != 0);

    for(int i = 0; i < 6; i++) mac[i] = inb(io_base + i);
    outl(io_base + 0x30, (uint32_t)&rx_buffer);
    outw(io_base + 0x3C, 0x0005);
    outl(io_base + 0x44, 0xf | (1 << 7));
    outb(io_base + 0x37, 0x0C);

    gateway_mac[0] = 0x52;
    gateway_mac[1] = 0x54;
    gateway_mac[2] = 0x00;
    gateway_mac[3] = 0x12;
    gateway_mac[4] = 0x35;
    gateway_mac[5] = 0x02;

printk(YELLOW, "[ NET ] Gateway MAC forzada a 52:54:00:12:35:02\n");
}

void send_arp_request(uint8_t ip1, uint8_t ip2, uint8_t ip3, uint8_t ip4) {
    uint8_t packet[64];
    mm_memset(packet, 0, 64);
    struct eth_header *eth = (struct eth_header *)packet;
    struct arp_packet *arp = (struct arp_packet *)(packet + 14);

    for(int i=0; i<6; i++) eth->dest[i] = 0xFF;
    for(int i=0; i<6; i++) eth->src[i] = mac[i];
    eth->type = htons(0x0806);

    arp->hw_type = htons(1);
    arp->proto_type = htons(0x0800);
    arp->hw_len = 6;
    arp->proto_len = 4;
    arp->opcode = htons(1);

    for(int i=0; i<6; i++) arp->src_mac[i] = mac[i];
    uint8_t *sip = (uint8_t*)&arp->src_ip;
    sip[0] = 10; sip[1] = 0; sip[2] = 2; sip[3] = 15;
    uint8_t *dip = (uint8_t*)&arp->dest_ip;
    dip[0] = ip1; dip[1] = ip2; dip[2] = ip3; dip[3] = ip4;

    outl(io_base + 0x20, (uint32_t)packet);
    outl(io_base + 0x10, 64);
}

void mini_curl(uint8_t ip1, uint8_t ip2, uint8_t ip3, uint8_t ip4) {
    if (gateway_mac[0] == 0 && gateway_mac[1] == 0) {
        printk(RED, "[ CURL ] Error: No MAC. Ejecuta 'arp 10.0.2.2' primero.\n");
        return;
    }
    uint8_t packet[74];
    mm_memset(packet, 0, 74);
    struct eth_header *eth = (struct eth_header *)packet;
    struct ip_header *ip = (struct ip_header *)(packet + 14);
    struct tcp_header *tcp = (struct tcp_header *)(packet + 34);

    for(int i=0; i<6; i++) eth->dest[i] = gateway_mac[i];
    for(int i=0; i<6; i++) eth->src[i] = mac[i];
    eth->type = htons(0x0800);

    ip->ver_ihl = 0x45;
    ip->proto = 6;
    ip->len = htons(40);
    ip->ttl = 64;
    uint8_t *sip = (uint8_t*)&ip->src_ip;
    sip[0] = 10; sip[1] = 0; sip[2] = 2; sip[3] = 15;
    uint8_t *dip = (uint8_t*)&ip->dest_ip;
    dip[0] = ip1; dip[1] = ip2; dip[2] = ip3; dip[3] = ip4;
    ip->checksum = net_checksum(ip, 20);

    tcp->src_port = htons(12345);
    tcp->dest_port = htons(80);
    tcp->seq_num = htonl(tcp_seq);
    tcp->ack_num = 0;
    tcp->data_offset = 0x50;
    tcp->flags = 0x02; // SYN
    tcp->window_size = htons(64240);

    outl(io_base + 0x20, (uint32_t)packet);
    outl(io_base + 0x10, 74);
    printk(CYAN, "[ CURL ] SYN Sent to %d.%d.%d.%d\n", ip1, ip2, ip3, ip4);
}

void send_ping(uint8_t ip1, uint8_t ip2, uint8_t ip3, uint8_t ip4) {
    uint8_t packet[64];
    mm_memset(packet, 0, 64);

    struct eth_header *eth = (struct eth_header *)packet;
    struct ip_header *ip = (struct ip_header *)(packet + 14);
    struct icmp_header *icmp = (struct icmp_header *)(packet + 34);

    for(int i=0; i<6; i++) eth->dest[i] = gateway_mac[0] == 0 ? 0xFF : gateway_mac[i];
    for(int i=0; i<6; i++) eth->src[i] = mac[i];
    eth->type = htons(0x0800);

    ip->ver_ihl = 0x45;
    ip->proto = 1; // ICMP
    ip->len = htons(40); // 20 IP + 20 ICMP
    ip->ttl = 64;
    uint8_t *sip = (uint8_t*)&ip->src_ip;
    sip[0] = 10; sip[1] = 0; sip[2] = 2; sip[3] = 15;
    uint8_t *dip = (uint8_t*)&ip->dest_ip;
    dip[0] = ip1; dip[1] = ip2; dip[2] = ip3; dip[3] = ip4;
    ip->checksum = net_checksum(ip, 20);

    icmp->type = 8; // Echo Request
    icmp->code = 0;
    icmp->id = htons(0x1337);
    icmp->seq = htons(1);
    icmp->checksum = net_checksum(icmp, 20);

    outl(io_base + 0x20, (uint32_t)packet);
    outl(io_base + 0x10, 64);
}
