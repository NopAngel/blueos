#ifndef BTRFS_H
#define BTRFS_H

#include <stdint.h>

/* --- Configuración del Sistema de Archivos --- */
#define BLOCK_SIZE         512
#define MAX_BLOCKS         1024
#define CACHE_SLOTS        32
#define MAX_INODES         64
#define BTRFS_NAME_MAX     32



/* --- Estructuras en Disco (Persistencia) --- */

struct vfs_node;

// Metadatos de un archivo o directorio
struct btrfs_disk_inode {
    uint32_t size;           // 4 bytes
    uint32_t block_ptrs[8];   // 32 bytes
    char name[28];           // 28 bytes -> Total = 64 bytes perfectos
};

// Entrada de directorio (para listar archivos)
struct btrfs_dir_entry {
    uint32_t inode_id;
    char name[BTRFS_NAME_MAX];
};

/* --- Estructuras en Memoria (Gestión) --- */

typedef struct {
    uint32_t block_id;
    uint8_t  data[512];
    bool     dirty;
    bool     valid;
} buffer_slot_t;

/* --- Prototipos de funciones (API de BlueFS) --- */

// Inicialización
void btrfs_init(void);

// Gestión de bloques y caché
void* block_get(uint32_t block_id);
void  block_mark_dirty(uint32_t block_id);
void  block_flush(void); // Escribir todo lo pendiente a disco

void btrfs_flush_cache();
// Gestión de inodos
struct btrfs_disk_inode* get_inode(uint32_t inode_id);
int btrfs_write_file(uint32_t inode_id, const char* content, uint32_t len);
int btrfs_create_file(struct vfs_node* parent, const char* name, const char* content);

// Gestión de Journaling
void journal_log(uint32_t block_id, void* data);
void btrfs_mount_disk();

#endif // BTRFS_H