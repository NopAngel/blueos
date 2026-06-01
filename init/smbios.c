#include <stdint.h>
#include <lib/string.h>
#include <kernel/printk.h>

struct smbios_entry {
    char anchor[4];
    uint8_t checksum;
    uint8_t length;
    uint8_t major;
    uint8_t minor;
    uint16_t max_struct_size;
    uint8_t revision;
    uint8_t formatted[5];
    char anchor2[5];
    uint8_t checksum2;
    uint16_t table_length;
    uint32_t table_address;
    uint16_t count;
    uint8_t bcd_revision;
} __attribute__((packed));

struct smbios_header {
    uint8_t type;
    uint8_t length;
    uint16_t handle;
} __attribute__((packed));

/* Necesitamos mapear la memoria física a virtual */
extern void vmm_map(uint32_t* virt, uint32_t phys, uint32_t size, uint32_t flags);

void get_machine_uuid(char *out) {
    /* El área 0xF0000 suele estar mapeada identity en kernels básicos, 
       pero si no, podrías necesitar vmm_map aquí también. */
    uint8_t *mem = (uint8_t *)0x000F0000; 
    struct smbios_entry *entry = 0;

    /* Buscamos la firma _SM_ */
    for (uint32_t i = 0; i < 0xFFF0; i += 16) {
        if (mem[i] == '_' && mem[i+1] == 'S' && mem[i+2] == 'M' && mem[i+3] == '_') {
            entry = (struct smbios_entry *)(mem + i);
            break;
        }
    }

    if (!entry) {
        strcpy(out, "Not Found");
        return;
    }

    /* IMPORTANTE: Mapear la tabla física a una dirección virtual temporal o fija */
    uint32_t* table_virt = (uint32_t*)0xE0100000; // Una dirección libre
    vmm_map(table_virt, entry->table_address, entry->table_length + 0x1000, 0x1 | 0x2);

    uint8_t *ptr = (uint8_t *)table_virt;
    for (int i = 0; i < entry->count; i++) {
        struct smbios_header *hdr = (struct smbios_header *)ptr;
        
        if (hdr->type == 1) { // System Information
            if (hdr->length < 0x19) break; // UUID no disponible en versiones viejas

            uint8_t *uuid = ptr + 8;
            sprintf(out, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7],
                uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
            return;
        }

        /* Saltar la parte formateada + la sección de strings */
        ptr += hdr->length;
        while (!(*ptr == 0 && *(ptr + 1) == 0)) {
            ptr++;
            if ((uint32_t)ptr > (uint32_t)table_virt + entry->table_length + 1024) break;
        }
        ptr += 2; // Saltar el doble null terminal
    }

    strcpy(out, "Unknown-UUID");
}

void get_machine_full_name(char *out) {
    // Aquí combinamos el nombre del OS con la arquitectura y versión
    #ifdef x86
        const char* arch = "x86_32";
    #elif defined(RISCV)
        const char* arch = "RISC-V";
    #else
        const char* arch = "Unknown";
    #endif

    sprintf(out, "BlueOS Professional Edition (%s)", arch);
}