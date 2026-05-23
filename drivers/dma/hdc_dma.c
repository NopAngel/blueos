#include <stdint.h>
#include <kernel/printk.h>
#include <kernel/colors.h>
#include <prdt.h>
#include <kernel/io.h>

#define BM_COMMAND_REG 0xC000
#define BM_STATUS_REG  0xC002
#define BM_PRDT_ADDR   0xC004

void hdcdma_init(void* buffer, uint32_t size) {
    printk(GREEN, "BlueOS: Enabling DMA Acceleration for HDC...\n");

    outb(BM_STATUS_REG, inb(BM_STATUS_REG) | 0x06);

    outl(BM_PRDT_ADDR, (uint32_t)&my_prdt);

    my_prdt[0].base_addr = (uint32_t)buffer; 
    my_prdt[0].byte_count = (uint16_t)size;
    my_prdt[0].last_entry = 1;

    outb(BM_COMMAND_REG, 0x08 | 0x01);
    printk(GREEN, "HDC: DMA ready for Bus Master transfer.\n");
}

void hdc_dma_read_start() {
    outb(BM_COMMAND_REG, 0x08); 

    // asm volatile("outb $0xC8, $0x1F7");

    outb(BM_COMMAND_REG, 0x08 | 0x01);
}


void wait_for_disk() {
    while (inb(0x1F7) & 0x80) {
        asm volatile("pause"); 
    }
}