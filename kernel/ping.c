#include <arch/x86/timer.h>
#include <drivers/virtio_net.h>
#include <kernel/net.h>
#include <kernel/printk.h>

struct icmp_packet {
  uint8_t type; // 8 = Echo Request
  uint8_t code; // 0
  uint16_t checksum;
  uint16_t id; // ID
  uint16_t seq;
  char payload[32];
} __attribute__((packed));

void run_real_ping() {
  struct icmp_packet ping;
  ping.type = 8;
  ping.code = 0;
  ping.id = 0x1337;
  ping.seq = 1;
  for (int i = 0; i < 32; i++)
    ping.payload[i] = 'A';
  ping.checksum = calculate_checksum(&ping, sizeof(ping));

  uint64_t start = timer_get_ticks();

  // submit
  virtio_net_send_packet(&ping, sizeof(ping));
  printk("\033[33mPING:\033[0m 10.0.2.2...\n");

  int received = 0;
  while ((timer_get_ticks() - start) < 2000) {
    if (virtio_net_poll_rx()) {
      received = 1;
      break;
    }
  }

  if (received) {
    uint64_t ms = timer_get_ticks() - start;
    printk("\033[32m[PING]\033[0m Reply desde 10.0.2.2: time=\033[36m%d "
           "ms\033[0m\n",
           (uint32_t)ms);
  } else {
    printk("\033[31m[PING]\033[0m Request timed out.\n");
  }
}

void do_ping(uint32_t ip) {
  uint64_t start = timer_get_ticks();

  while (virtio_net_poll_rx() == 0) {
    if ((timer_get_ticks() - start) > 2000) {
      printk("\033[31m[PING]\033[0m Timeout!\n");
      return;
    }
  }

  uint64_t total = timer_get_ticks() - start;
  printk("\033[36m[PING]\033[0m Respuesta en %d ms\n", (uint32_t)total);
}
