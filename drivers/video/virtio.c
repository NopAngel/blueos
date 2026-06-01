#include <drivers/virtio.h>
#include <kernel/ports.h>
#include <mm/memory.h>
#include <kernel/printk.h>

static uint32_t io_base = 0;
static vring_desc_t* desc;
static vring_avail_t* avail;
static vring_used_t* used;

void virtio_console_init(uint32_t base) {
    io_base = base;

    outb(io_base + 0x12, 0);

    outb(io_base + 0x12, 0x01 | 0x02);

    desc = (vring_desc_t*)kmalloc(4096);
    avail = (vring_avail_t*)kmalloc(4096);
    used = (vring_used_t*)kmalloc(4096);

    outw(io_base + 0x14, 0);

    outl(io_base + 0x18, (uint32_t)desc / 4096);
    outl(io_base + 0x1c, (uint32_t)avail / 4096);
    outl(io_base + 0x20, (uint32_t)used / 4096);

    // Driver OK
    outb(io_base + 0x12, 0x01 | 0x02 | 0x08);

    printk("VirtIO: Init. base: 0x%x\n", base);
}

void virtio_console_send(const char* buf, uint32_t len) {
    desc[0].addr = (uint64_t)(uint32_t)buf;
    desc[0].len = len;
    desc[0].flags = 0;
    desc[0].next = 0;

    avail->ring[avail->idx % 16] = 0;
    avail->idx++;

    outw(io_base + 0x10, 0);
}
