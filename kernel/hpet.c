#include <stdint.h>
#include <hpet.h>
#include <multiboot.h>
#include <kernel/printk.h>
#include <kernel/colors.h>
uint64_t hpet_base;
static uint32_t period_fs = 0;

void hpet_init(hpet_table_t* table) {
    hpet_base = table->base_address.address;

    uint64_t caps = *(volatile uint64_t*)(hpet_base + 0x00);
    uint32_t period = caps >> 32; 
    *(volatile uint64_t*)(hpet_base + 0x10) |= 0x03; 

    *(volatile uint64_t*)(hpet_base + 0xf0) = 0;
}

uint64_t hpet_get_nanos() {
    uint64_t counter = *(volatile uint64_t*)(hpet_base + 0xf0);

    return counter * (period_fs / 1000000);
}

hpet_table_t* find_hpet_table(multiboot_info_t* mbi) {

    char* rsdp_search = (char*)0x000E0000;
    uint32_t rsdp_addr = 0;

    for (uint32_t i = 0; i < 0x20000; i += 16) {
        if (*(uint64_t*)(rsdp_search + i) == 0x2052545020445352) { 
            rsdp_addr = (uint32_t)(rsdp_search + i);
            break;
        }
    }

    if (!rsdp_addr) return 0;


    uint32_t rsdt_addr = *(uint32_t*)(rsdp_addr + 16);
    struct acpi_sdt_header* rsdt = (struct acpi_sdt_header*)rsdt_addr;

    uint32_t entries = (rsdt->length - sizeof(struct acpi_sdt_header)) / 4;
    uint32_t* table_ptr = (uint32_t*)(rsdt_addr + sizeof(struct acpi_sdt_header));

    for (uint32_t i = 0; i < entries; i++) {
        struct acpi_sdt_header* h = (struct acpi_sdt_header*)table_ptr[i];
        if (h->signature[0] == 'H' && h->signature[1] == 'P' && 
            h->signature[2] == 'E' && h->signature[3] == 'T') {
            return (hpet_table_t*)h;
        }
    }
    return 0;
}