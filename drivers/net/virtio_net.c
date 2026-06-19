#include <drivers/virtio_net.h>
#include <kernel/ports.h>
#include <kernel/printk.h>
#include <mm/memory.h>

typedef struct {
  uint64_t addr;
  uint32_t len;
  uint16_t flags;
  uint16_t next;
} vring_desc_t;

static uint32_t net_base;
static vring_desc_t *tx_desc;
static uint16_t *tx_avail_ring;
static uint32_t *tx_idx;

typedef struct {
  uint32_t id;
  uint32_t len;
} vring_used_elem_t;

typedef struct {
  uint16_t flags;
  uint16_t idx;
  struct {
    uint32_t id;
    uint32_t len;
  } ring[];
} __attribute__((packed)) vring_used_t;

static vring_used_t *rx_used;
static uint16_t last_seen_idx = 0;

int virtio_net_poll_rx(void) {

  if (rx_used->idx != last_seen_idx) {
    last_seen_idx++;

    return 1;
  }
  return 0;
}

void virtio_net_init(uint32_t base) {
  net_base = base;

  outb(net_base + 0x12, 0);

  //  ACK and DRIVER
  outb(net_base + 0x12, 0x01 | 0x02);

  tx_desc = (vring_desc_t *)kmalloc(4096);

  outw(net_base + 0x14, 1);
  outl(net_base + 0x18, (uint32_t)tx_desc / 4096);

  outb(net_base + 0x12, 0x01 | 0x02 | 0x08);
  printk("virtIO: init\n");
}

void virtio_net_send_packet(void *packet, uint32_t len) {

  virtio_net_hdr_t hdr = {0};

  tx_desc[0].addr = (uint64_t)(uint32_t)&hdr;
  tx_desc[0].len = sizeof(hdr);
  tx_desc[0].flags = 2; // VRING_DESC_F_NEXT
  tx_desc[0].next = 1;

  tx_desc[1].addr = (uint64_t)(uint32_t)packet;
  tx_desc[1].len = len;
  tx_desc[1].flags = 0;

  outw(net_base + 0x10, 1);
}
