#include <fs/vfs.h>
#include <fs/initramfs.h>
#include <kernel/printk.h>
#include <lib/string.h>

extern void *kmalloc(uint32_t size);
extern void *mm_memset(void *s, int c, size_t n);

extern vfs_ops_t root_ops;

#define MODULE_NAME "INITRAMFS"

/* Estructura del header de un archivo TAR (ustar) */
typedef struct {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char checksum[8];
    char typeflag;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char padding[12];
} tar_header_t;

/* Convertidor octal a entero */
static uint32_t octal_to_int(const char *str, int max_len) {
    uint32_t result = 0;
    for (int i = 0; i < max_len && str[i] != '\0' && str[i] != ' '; i++) {
        result = (result << 3) + (str[i] - '0');
    }
    return result;
}

void ensure_directory_exists(const char *path) {
    char temp[256];
    strcpy(temp, path);
    char *ptr = temp + 1; 

    while ((ptr = strchr(ptr, '/')) != NULL) {
        *ptr = '\0';
        
        if (vfs_lookup(temp) == NULL) {
            vfs_mkdir(temp, 0755);
        }
        
        *ptr = '/';
        ptr++;
    }
}

int initrd_read(vfs_node_t *node, char *buffer, uint32_t size, uint32_t offset) {
    uint32_t initrd_base = 0x1000000; 

    char *data_ptr = (char *)(initrd_base + node->inode + offset);
    
    
    memcpy(buffer, data_ptr, size);
    return size;
}

void initrd_init(uint32_t location) {
    uint32_t offset = 0;
    vfs_node_t *root = vfs_get_root(); 

    while (1) {
        tar_header_t *header = (tar_header_t *)(location + offset);
        if (header->name[0] == '\0') break;

        uint32_t size = octal_to_int(header->size, 12); 

        vfs_node_t *node = (vfs_node_t *)kmalloc(sizeof(vfs_node_t));
        mm_memset(node, 0, sizeof(vfs_node_t)); 
        
        strncpy(node->name, header->name, 128); 
        node->size = size;
        node->type = VFS_TYPE_FILE;
        node->inode = offset + 512; 
        
        extern vfs_ops_t root_ops;
        node->ops = &root_ops;    
        
        node->parent = root;

        if (root) {
            node->next = root->ptr;
            root->ptr = node;
        }

        offset += ((size + 511) & ~511) + 512;
    }
}

void initramfs_parse(uintptr_t ramdisk_start, uintptr_t ramdisk_end) {
    uintptr_t address = ramdisk_start;
    int files_loaded = 0;

    boot_msg(MODULE_NAME, "Unpacking ramdisk image...\n", 0);

    while (address < ramdisk_end) {
        tar_header_t *header = (tar_header_t *)address;

        if (header->name[0] == '\0') break;

        uint32_t file_size = octal_to_int(header->size, 12);
        void *file_data = (void *)(address + 512);

        char full_path[256];
        sprintf(full_path, "/%s", header->name);

        int len = strlen(full_path);
        if (len > 1 && full_path[len - 1] == '/') {
            full_path[len - 1] = '\0';
        }

        if (header->typeflag == '5') {
            if (vfs_lookup(full_path) == NULL) {
                vfs_mkdir(full_path, 0755);
                printk("<6>[  %s   ] Dir: %s\n", MODULE_NAME, full_path);
            } else {
                printk("<6>[  %s   ] Dir: %s (Already exists, skipping creation)\n", MODULE_NAME, full_path);
            }
        }

        else if (header->typeflag == '0' || header->typeflag == '\0') {
    ensure_directory_exists(full_path);
    
    if (vfs_lookup(full_path) == NULL) {
        vfs_create(full_path, NULL); 

        vfs_node_t *new_node = vfs_lookup(full_path);
        if (new_node) {
            new_node->ptr = (void *)file_data; 
            
            new_node->size = file_size; 

            extern int xfs_bind_file_buffer(uint32_t inode_num, uint32_t size, uintptr_t ram_address);
            xfs_bind_file_buffer(new_node->inode, file_size, (uintptr_t)file_data);
        }
        printk("<6>[  INITRAMFS   ] File: %s (%u bytes) mapped successfully.\n", full_path, file_size);
    }
}

        address += 512 + ((file_size + 511) & ~511);
        files_loaded++;
    }
    printk("<6>[  %s   ] Initrd ready. %d assets loaded.\n", MODULE_NAME, files_loaded);
}