#include <fs/vfs.h>
#include <fs/xfs.h>
#include <kernel/errno.h>
#include <kernel/printk.h>
#include <lib/string.h>

extern void *kmalloc(uint32_t size);

#define MODULE_NAME "XFS_CORE"

#ifndef ENOSPC
#define ENOSPC 28
#endif
#ifndef EINVAL
#define EINVAL 22
#endif
#ifndef EIO
#define EIO 5
#endif

xfs_ag_t g_xfs_groups[XFS_AG_COUNT];
static uint32_t g_global_inode_counter = 100;

extern int xfs_pick_ag(void);
extern int32_t xfs_alloc_block(int ag_id);

/**
 * xfs_init: Inicializa las Allocation Groups autónomas.
 */
void xfs_init(void) {
  for (int i = 0; i < XFS_AG_COUNT; i++) {
    g_xfs_groups[i].ag_id = i;
    g_xfs_groups[i].free_blocks = 1024;
    g_xfs_groups[i].inode_count = 0;
    memset(g_xfs_groups[i].block_bitmap, 0,
           sizeof(g_xfs_groups[i].block_bitmap));

    for (int j = 0; j < XFS_MAX_FILES_AG; j++) {
      g_xfs_groups[i].inodes[j].is_active = 0;
    }
  }
  printk("<6>[  %s   ] BlueXFS Engine online. %d Autonomous Allocation Groups active.\n",
         MODULE_NAME, XFS_AG_COUNT);
}

/**
 * xfs_create_file: Provisiona metadatos y amarra los nodos al i-nodo padre.
 */
int xfs_create_file(const char *name, uint32_t type, uint32_t parent_inode) {
    if (!name || strlen(name) >= XFS_MAX_NAME) return -EINVAL;
    
    /* --- LIMPIEZA DE CADENAS (ANTI-FANTASMAS DEL TAR) --- */
    char clean_name[XFS_MAX_NAME];
    int k = 0;
    
    /* Saltamos una barra inicial si existe */
    int start_idx = (name[0] == '/') ? 1 : 0;
    
    for (int i = start_idx; name[i] != '\0' && k < (XFS_MAX_NAME - 1); i++) {
        /* Si encontramos un espacio o carácter de control residual del TAR, cortamos */
        if (name[i] == ' ' || name[i] == '\t' || name[i] == '\n' || name[i] == '\r') {
            break;
        }
        clean_name[k++] = name[i];
    }
    clean_name[k] = '\0'; /* Clausuramos el string de forma segura */

    /* Si el nombre quedó vacío tras la limpieza (ej: la raíz '/'), salimos airosos */
    if (k == 0) {
        return 0; 
    }

    /* Asignación dinámica de la Allocation Group */
    int ag_id = xfs_pick_ag();
    xfs_ag_t *ag = &g_xfs_groups[ag_id];
    
    if (ag->inode_count >= XFS_MAX_FILES_AG) {
        printk("<3>[  %s   ] Allocation error: No free inode records left inside AG %d\n", MODULE_NAME, ag_id);
        return -ENOSPC;
    }
    
    int32_t assigned_block = xfs_alloc_block(ag_id);
    if (assigned_block < 0) return -ENOSPC;

    /* Buscamos una ranura de i-nodo libre dentro del AG asignado */
    for (int i = 0; i < XFS_MAX_FILES_AG; i++) {
        if (!ag->inodes[i].is_active) {
            xfs_inode_t *ni = &ag->inodes[i];
            ni->inode_num = g_global_inode_counter++;
            
            strcpy(ni->name, clean_name);
            ni->size = 0;
            ni->type = type;
            ni->start_block = (uint32_t)assigned_block;
            ni->parent_inode = parent_inode; /* Guardamos el i-nodo del directorio creador */
            ni->is_active = 1;
            
            ag->inode_count++;
            printk("<6>[  %s   ] Target entry '%s' bound to Inode %u inside isolated AG %d (Parent Inode: %u, Block Map: %d)\n", 
                   MODULE_NAME, clean_name, ni->inode_num, ag_id, parent_inode, assigned_block);
            return 0;
        }
    }
    return -EIO;
}

int xfs_vfs_mkdir(struct vfs_node *node, const char *name, uint16_t mode) {
    (void)mode;
    uint32_t p_inode = node ? node->inode : 0; 
    return xfs_create_file(name, VFS_TYPE_DIR, p_inode); 
}

int xfs_vfs_create(struct vfs_node *node, const char *name, const char *flags) {
    (void)flags;
    uint32_t p_inode = node ? node->inode : 0;
    return xfs_create_file(name, VFS_TYPE_FILE, p_inode); 
}

void xfs_ls(void) {
  printk("\n--- XFS Unified Storage Tree Mapping ---\n");
  for (int i = 0; i < XFS_AG_COUNT; i++) {
    printk("Allocation Group %d Status [%d/1024 Free Blocks]:\n",
           g_xfs_groups[i].ag_id, g_xfs_groups[i].free_blocks);

    int printed_inodes = 0;
    for (int j = 0; j < XFS_MAX_FILES_AG; j++) {
      if (g_xfs_groups[i].inodes[j].is_active) {
        xfs_inode_t *node = &g_xfs_groups[i].inodes[j];

        printk("  -> INODE: %u | NAME: %s | TYPE: %s | PARENT: %u | RAW_BLOCK: %u\n",
               node->inode_num, node->name,
               (node->type == VFS_TYPE_DIR) ? "DIR" : "FILE",
               node->parent_inode, node->start_block);

        printed_inodes++;
      }
    }
    if (printed_inodes == 0) {
      printk("  (Empty allocation records layout cluster)\n");
    }
  }
  printk("----------------------------------------\n\n");
}

int xfs_vfs_readdir(struct vfs_node *node, uint32_t index, void *dirent_out) {
    struct vfs_dirent *de = (struct vfs_dirent *)dirent_out;
    uint32_t current_idx = 0;
    
    uint32_t target_parent = node ? node->inode : 0; 

    for (int i = 0; i < XFS_AG_COUNT; i++) {
        for (int j = 0; j < XFS_MAX_FILES_AG; j++) {
            if (g_xfs_groups[i].inodes[j].is_active && 
                g_xfs_groups[i].inodes[j].parent_inode == target_parent) {
                
                if (current_idx == index) {
                    xfs_inode_t *ni = &g_xfs_groups[i].inodes[j];
                    strncpy(de->name, ni->name, VFS_NAME_MAX);
                    de->type = ni->type;
                    de->inode = ni->inode_num;
                    return 0; 
                }
                current_idx++;
            }
        }
    }
    return -1; 
}

/**
 * xfs_vfs_read - Reads data from a node directly mapped into the RAM initial ramdisk
 * @node: Target virtual filesystem node
 * @buffer: Destination memory storage buffer
 * @size: Total bytes requested to be read
 * @offset: Reading displacement pointer offset
 * * Returns: Number of successful bytes retrieved into the buffer, negative on error.
 */
int xfs_vfs_read(struct vfs_node *node, char *buffer, uint32_t size, uint32_t offset) {
    if (!node || !buffer) return -1;
    if (size == 0) return 0;

    uint32_t final_size = node->size;
    uintptr_t final_ram = node->ram_address;

    if (final_ram == 0) {
        for (int i = 0; i < XFS_AG_COUNT; i++) {
            for (int j = 0; j < XFS_MAX_FILES_AG; j++) {
                if (g_xfs_groups[i].inodes[j].is_active && 
                    g_xfs_groups[i].inodes[j].inode_num == node->inode) {
                    final_size = g_xfs_groups[i].inodes[j].size;
                    final_ram = g_xfs_groups[i].inodes[j].ram_address;
                    break;
                }
            }
        }
    }

    /* Check boundaries against true resolved asset file measurements */
    if (offset >= final_size) return 0;
    if (offset + size > final_size) {
        size = final_size - offset;
    }

    /* Secure data buffer streaming execution hook */
    if (final_ram != 0) {
        memcpy(buffer, (void *)(final_ram + offset), size);
        return size;
    }

    return 0; 
}

/**
 * xfs_vfs_finddir: Resuelve rutas de navegación abstrayendo el i-nodo a un nodo VFS.
 */
vfs_node_t *xfs_vfs_finddir(struct vfs_node *parent, const char *name) {
    if (!name) return NULL;
    
    uint32_t target_parent = parent ? parent->inode : 0;

    for (int i = 0; i < XFS_AG_COUNT; i++) {
        for (int j = 0; j < XFS_MAX_FILES_AG; j++) {
            if (g_xfs_groups[i].inodes[j].is_active && 
                g_xfs_groups[i].inodes[j].parent_inode == target_parent) {
                
                xfs_inode_t *ni = &g_xfs_groups[i].inodes[j];
                if (strcmp(ni->name, name) == 0) {
                    vfs_node_t *node = (vfs_node_t *)kmalloc(sizeof(vfs_node_t));
                    if (!node) return NULL;
                    
                    memset(node, 0, sizeof(vfs_node_t));
                    strncpy(node->name, ni->name, VFS_NAME_MAX);
                    node->inode = ni->inode_num;
                    node->size = ni->size;
                    
                    /* FIXED: Inherit the physical ram data pointer from the persistent XFS storage cache */
                    node->ram_address = ni->ram_address; 
                    
                    node->parent = (vfs_node_t *)parent;
                    node->ops = parent->ops;
                    node->type = (ni->type == VFS_TYPE_DIR) ? VFS_TYPE_DIR : VFS_TYPE_FILE;
                    
                    return node;
                }
            }
        }
    }
    return NULL;
}

int xfs_write_file_size(uint32_t inode_num, uint32_t size) {
    for (int i = 0; i < XFS_AG_COUNT; i++) {
        for (int j = 0; j < XFS_MAX_FILES_AG; j++) {
            if (g_xfs_groups[i].inodes[j].is_active && 
                g_xfs_groups[i].inodes[j].inode_num == inode_num) {
                
                g_xfs_groups[i].inodes[j].size = size;
                return 0; 
            }
        }
    }
    return -1; // NOT FOUND
}
int xfs_bind_file_buffer(uint32_t inode_num, uint32_t size, uintptr_t ram_address) {
    for (int i = 0; i < XFS_AG_COUNT; i++) {
        for (int j = 0; j < XFS_MAX_FILES_AG; j++) {
            if (g_xfs_groups[i].inodes[j].is_active && 
                g_xfs_groups[i].inodes[j].inode_num == inode_num) {
                
                g_xfs_groups[i].inodes[j].size = size;
                g_xfs_groups[i].inodes[j].ram_address = ram_address; /* Persistent binding hook */
                return 0;
            }
        }
    }
    return -1;
}

/* Tabla global de operaciones apuntando a las funciones jerárquicas */
vfs_ops_t xfs_ops = {
    .mkdir   = xfs_vfs_mkdir,
    .create  = xfs_vfs_create,
    .readdir = (void *)xfs_vfs_readdir,
    .finddir = (void *)xfs_vfs_finddir,
    .read    = (void *)xfs_vfs_read
};

void xfs_mount(struct vfs_node *mount_point) {
  if (!mount_point)
    return;
  mount_point->ops = &xfs_ops;
  printk("[ XFS ] Successfully mounted XFS instance onto VFS node structure.\n");
}