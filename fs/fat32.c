#include <stdint.h>
#include <blueos/printk.h>
#include <fs/fat32.h>      // ¡AQUÍ ESTÁN LOS STRUCTS!
#include <blueos/colors.h>

uint32_t cluster_to_lba(uint32_t cluster);
uint32_t get_fat_entry(uint32_t cluster);
void ata_read_sector(uint32_t lba, uint8_t *buffer); // Declarar driver ATA
void mm_memcpy(void *dest, const void *src, uint32_t n); // Declarar memcpy
void read_sector(uint32_t lba, uint8_t *buffer);
// --- VARIABLES GLOBALES ---
struct fat32_bpb bpb;
uint32_t current_dir_cluster = 0;
char current_path[256] = "/";
void command_pwd() {
    printk(WHITE, "%s\n", current_path);
}

void init_fat32() {
    uint8_t buffer[512];
    
    // Leemos el sector 0 del disco principal
    ata_read_sector(0, buffer);
    mm_memcpy(&bpb, buffer, sizeof(struct fat32_bpb));

    // Verificación de seguridad
    if (bpb.boot_signature != 0x29) {
        printk(RED, "[ FAT32 ] Error: Firma de boot no encontrada.\n");
        return;
    }

    printk(GREEN, "[ FAT32 ] Sistema montado: %s\n", bpb.volume_label);
    printk(CYAN, "  Clusters de %d sectores\n", bpb.sectors_per_cluster);
    printk(CYAN, "  Root Cluster: %d\n", bpb.root_cluster);
}


void fat32_ls(uint32_t cluster) {
    uint8_t buffer[512]; // Asumimos sectores de 512 bytes
    uint32_t lba = cluster_to_lba(cluster); // Función que debes crear (ver abajo)
    
    ata_read_sector(lba, buffer);
    
    struct fat32_directory_entry *entry = (struct fat32_directory_entry *)buffer;

    printk(CYAN, "Nombre          Tipo    Tamaño\n");
    printk(BLUE, "------------------------------------\n");

    for (int i = 0; i < 16; i++) { // 16 entradas por sector (512 / 32)
        if (entry[i].name[0] == 0x00) break;  // No hay más archivos
        if (entry[i].name[0] == 0xE5) continue; // Archivo borrado

        // Si es un archivo normal o directorio
        if (entry[i].attributes & 0x0F) { 
            // Formatear nombre (Quitar espacios)
            for(int j=0; j<8; j++) if(entry[i].name[j] != ' ') printk(WHITE, "%c", entry[i].name[j]);
            if(entry[i].ext[0] != ' ') {
                printk(WHITE, ".");
                for(int j=0; j<3; j++) if(entry[i].ext[j] != ' ') printk(WHITE, "%c", entry[i].ext[j]);
            }

            if (entry[i].attributes & 0x10) {
                printk(GREEN, "    <DIR>");
            } else {
                printk(YELLOW, "    <FILE>  %d bytes", entry[i].file_size);
            }
            printk(WHITE, "\n");
        }
    }
}


uint32_t cluster_to_lba(uint32_t cluster) {
    // Esta es la fórmula estándar de FAT32
    uint32_t first_data_sector = bpb.reserved_sectors + (bpb.fat_count * bpb.fat_size_32);
    return first_data_sector + ((cluster - 2) * bpb.sectors_per_cluster);
}



void fat32_cat(uint32_t start_cluster, uint32_t file_size) {
    uint8_t buffer[512];
    uint32_t current_cluster = start_cluster;
    uint32_t bytes_left = file_size;

    while (bytes_left > 0 && current_cluster < 0x0FFFFFF8) {
        uint32_t lba = cluster_to_lba(current_cluster);
        
        // Leer los sectores del cluster (asumiendo 1 sector por cluster para simplificar)
        ata_read_sector(lba, buffer);
        
        // Cuántos bytes imprimir de este sector
        uint32_t to_read = (bytes_left > 512) ? 512 : bytes_left;
        
        for (uint32_t i = 0; i < to_read; i++) {
            printk(WHITE, "%c", buffer[i]);
        }

        // Consultar la tabla FAT para el siguiente cluster
        current_cluster = get_fat_entry(current_cluster);
        bytes_left -= to_read;
    }
    printk(WHITE, "\n");
}

uint32_t get_fat_entry(uint32_t cluster) {
    uint8_t fat_buffer[512];
    // Calcular en qué sector de la FAT está la entrada del cluster
    uint32_t fat_sector = bpb.reserved_sectors + ((cluster * 4) / 512);
    uint32_t fat_offset = (cluster * 4) % 512;

    ata_read_sector(fat_sector, fat_buffer);
    
    // Devolvemos los 4 bytes (limpiando los 4 bits de arriba que son reservados)
    return (*(uint32_t*)&fat_buffer[fat_offset]) & 0x0FFFFFFF;
}