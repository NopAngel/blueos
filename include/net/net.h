// include/net/net.h
typedef struct {
    unsigned char dest_mac[6];
    unsigned char src_mac[6];
    unsigned short type; // 0x0800 para IPv4, 0x0806 para ARP
} __attribute__((packed)) ethernet_frame_t;

typedef struct {
   
    unsigned char protocol; 
    unsigned int src_ip;
    unsigned int dest_ip;
} __attribute__((packed)) ipv4_packet_t;