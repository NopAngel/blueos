#include <blueos/ports.h>
#include <string.h>
#include <blueos/printk.h>
#include <blueos/colors.h>

struct rsdp_ptr {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_address;
} __attribute__((packed));

struct acpi_header {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((packed));

struct fadt_table {
    struct acpi_header header;
    uint32_t firmware_ctrl;
    uint32_t dsdt;
    uint8_t  reserved;
    uint8_t  preferred_pm_profile;
    uint16_t sci_int;
    uint32_t smi_cmd;
    uint8_t  acpi_enable;
    uint8_t  acpi_disable;
    uint32_t pm1a_cnt_blk; 
} __attribute__((packed));

uint32_t PM1a_CNT_BLK;

void acpi_init() {
    struct rsdp_ptr* rsdp = NULL;
    for (uint32_t addr = 0x000E0000; addr < 0x000FFFFF; addr += 16) {
        if (memcmp((void*)addr, "RSD PTR ", 8) == 0) {
            rsdp = (struct rsdp_ptr*)addr;
            break;
        }
    }

    if (!rsdp) {
        printk(RED, "ACPI: RSDP no encontrada\n");
        return;
    }

    struct acpi_header* rsdt = (struct acpi_header*)rsdp->rsdt_address;
    

    uint32_t* entry = (uint32_t*)(rsdp->rsdt_address + sizeof(struct acpi_header));
    uint32_t entries = (rsdt->length - sizeof(struct acpi_header)) / 4;

    for (uint32_t i = 0; i < entries; i++) {
        uint32_t* h = (uint32_t*)entry[i];
        if (memcmp(h, "FACP", 4) == 0) { 
          
            uint32_t fadt_addr = entry[i];
            PM1a_CNT_BLK = *(uint32_t*)(fadt_addr + 64); 
            
            printk(GREEN, "ACPI: FADT encontrada en 0x%x\n", fadt_addr);
            printk(GREEN, "ACPI: Puerto real detectado: 0x%x\n", PM1a_CNT_BLK);
            return;
        }
    }
}

void sys_shutdown() {
    printk(RED, "BlueOS: Ejecutando secuencia final de apagado...\n");


    outw(0x604, 0x2000 | 0x08); 
    outb(0x501, 0x00); 

    outw(0x4004, 0x3400);

    printk(RED, "Sistema detenido. Puedes cerrar la ventana.\n");
    while(1) {
        __asm__ __volatile__("cli; hlt");
    }
}

void sys_reboot() {
    printk(YELLOW, "\nBlueOS: Reiniciando sistema...\n");

  
    while ((inb(0x64) & 0x02) != 0);

    outb(0x64, 0xFE);

   
    printk(RED, "Reinicio por 8042 falló. Intentando Triple Fault...\n");
    

    struct {
        uint16_t limit;
        uint32_t base;
    } __attribute__((packed)) idt_zero = {0, 0};

    __asm__ __volatile__("lidt %0; int3" : : "m"(idt_zero));

    while(1) {
        __asm__ __volatile__("cli; hlt");
    }
}