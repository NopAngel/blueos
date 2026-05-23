#include <kernel/net.h>
#include <lib/string.h>

/* Simple packet pool for BlueOS */
static struct net_packet packet_pool[16]; 
static int packet_in_use[16] = {0};

struct net_packet* net_alloc_packet() {
    for (int i = 0; i < 16; i++) {
        if (!packet_in_use[i]) {
            packet_in_use[i] = 1;
            memset(&packet_pool[i], 0, sizeof(struct net_packet));
            return &packet_pool[i];
        }
    }
    return (struct net_packet*)0; // No packets available
}

void net_free_packet(struct net_packet *pkt) {
    for (int i = 0; i < 16; i++) {
        if (&packet_pool[i] == pkt) {
            packet_in_use[i] = 0;
            return;
        }
    }
}

int net_transmit(struct net_packet *pkt) {
    /* * HERE: You would call your physical NIC driver (e.g., VirtIO-Net or E1000)
     * For now, we just print and free the packet.
     */
    // printk("[NET] Transmitting packet of length: %d\n", pkt->len);
    
    net_free_packet(pkt);
    return 0;
}