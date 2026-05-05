#include <fs/fat16.h>
#include <kernel/malloc.h>
#include <drivers/ata.h> // O
#include <lib/string.h>
#include <kernel/printk.h>
#include <kernel/malloc.h>

uint8_t *sector_buffer = NULL;
extern void kfree(void* ptr);
/**
 * to_fat_name: Convierte "test.txt" a "TEST    TXT"
 */
static void to_fat_name(const char* filename, char* out) {
    memset(out, ' ', 11);
    int i = 0, j = 0;
    while (filename[i] && filename[i] != '.' && j < 8) {
        out[j++] = (filename[i] >= 'a' && filename[i] <= 'z') ? filename[i] - 32 : filename[i];
        i++;
    }
    if (filename[i] == '.') {
        i++;
        j = 8;
        while (filename[i] && j < 11) {
            out[j++] = (filename[i] >= 'a' && filename[i] <= 'z') ? filename[i] - 32 : filename[i];
            i++;
        }
    }
}

/**
 * fat16_read_file: Busca y carga un archivo en un buffer
 */
uint8_t* fat16_read_file(fat16_bpb_t* bpb, const char* filename) {
    char fat_name[11];
    to_fat_name(filename, fat_name);

    uint32_t root_dir_sector = bpb->reserved_sectors + (bpb->fat_count * bpb->fat_size_16);
    uint32_t root_dir_size = (bpb->root_entries * 32) / bpb->bytes_per_sector;

    fat_entry_t* entries = (fat_entry_t*)kmalloc(bpb->bytes_per_sector);
    if (sector_buffer == NULL) {
    	sector_buffer = kmalloc(512);
    }
    for (uint32_t s = 0; s < root_dir_size; s++) {
        ata_read_sector(root_dir_sector + s, (uint8_t*)entries);

        for (int e = 0; e < (bpb->bytes_per_sector / 32); e++) {
            if (entries[e].name[0] == 0x00) break;
            if (entries[e].name[0] == 0xE5) continue;

            if (memcmp(entries[e].name, fat_name, 11) == 0) {
                // ¡Encontrado!
                uint32_t file_size = entries[e].file_size;
                uint16_t current_cluster = entries[e].cluster_low;

                uint8_t* file_buffer = (uint8_t*)kmalloc(file_size);

                uint32_t data_region_start = root_dir_sector + root_dir_size;


                uint32_t sector = ((current_cluster - 2) * bpb->sectors_per_cluster) + data_region_start;

                for (uint32_t i = 0; i <= (file_size / bpb->bytes_per_sector); i++) {
                    ata_read_sector(sector + i, file_buffer + (i * bpb->bytes_per_sector));
                }

                kfree(entries);
                return file_buffer;
            }
        }
    }

    kfree(entries);
    return NULL;
}
