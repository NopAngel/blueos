#include "../include/fs/vfs.h"
#include "../include/fs/fs.h"
#include <blueos/colors.h>

extern int cursor_y;
extern unsigned int directory_count;
extern unsigned int file_count;
extern DirectoryEntry directory_table[MAX_DIRECTORIES];
extern FileEntry file_table[MAX_FILES];


vfs_node_t vfs_nodes[MAX_VFS_NODES];
int total_vfs_nodes = 0;

static vfs_system vfs;
static vfs_file_handle open_files[VFS_MAX_OPEN_FILES];
static char current_path[VFS_MAX_PATH] = "/";

static char read_buffer[VFS_MAX_CONTENT];

static char data_blocks[VFS_MAX_ENTRIES][VFS_MAX_CONTENT];

vfs_system* get_vfs_instance(void) {
    return &vfs;
}

#define MAX_DRIVERS 4
static struct vfs_driver *registered_drivers[MAX_DRIVERS];
static int num_registered_drivers = 0;

void vfs_register_driver(struct vfs_driver *driver) {
    if (num_registered_drivers < MAX_DRIVERS) {
        registered_drivers[num_registered_drivers] = driver;
        num_registered_drivers++;
        printk(GREEN, "VFS: Registered driver [%s]\n", driver->name);
    }
}

void vfs_memset(void *ptr, char value, unsigned int size) {
    char *p = (char *)ptr;
    for (unsigned int i = 0; i < size; i++) {
        p[i] = value;
    }
}

void vfs_memcpy(void *dest, const void *src, unsigned int size) {
    char *d = (char *)dest;
    const char *s = (const char *)src;
    for (unsigned int i = 0; i < size; i++) {
        d[i] = s[i];
    }
}

void vfs_strcat(char *dest, const char *src) {
    while (*dest != '\0') {
        dest++;
    }
    
    while (*src != '\0') {
        *dest = *src;
        dest++;
        src++;
    }
    
    *dest = '\0';
}

void vfs_init(void) {
    vfs_memset(vfs.entries, 0, sizeof(vfs.entries));
    vfs_memset(open_files, 0, sizeof(open_files));
    vfs_memset(data_blocks, 0, sizeof(data_blocks));
    
    vfs.entry_count = 0;
    

    vfs.entries[0].inode = 0;
    strcpy(vfs.entries[0].name, "/");
    vfs.entries[0].type = VFS_TYPE_DIRECTORY;
    vfs.entries[0].parent = 0;  
    vfs.entries[0].size = 0;
    vfs.entries[0].data_block = 0;
    vfs.entries[0].created_time = 0;
    vfs.entries[0].modified_time = 0;
    
    vfs.entry_count = 1;
    vfs.current_directory = 0;
    vfs.root_directory = 0;
}

static unsigned int vfs_allocate_inode(void) {
    if (vfs.entry_count >= VFS_MAX_ENTRIES) {
        return (unsigned int)-1;
    }
    return vfs.entry_count++;
}

static vfs_entry* vfs_find_in_directory(unsigned int dir_inode, const char *name, vfs_entry_type type) {
    if (dir_inode >= vfs.entry_count) {
        return NULL;
    }
    
    for (unsigned int i = 0; i < vfs.entry_count; i++) {
        if (vfs.entries[i].parent == dir_inode && 
            vfs.entries[i].type == type &&
            strcmp(vfs.entries[i].name, name) == 0) {
            return &vfs.entries[i];
        }
    }
    return NULL;
}

static int vfs_add_entry(vfs_entry *entry) {
    if (vfs.entry_count >= VFS_MAX_ENTRIES) {
        return -1;
    }
    

    vfs_entry *existing = vfs_find_in_directory(entry->parent, entry->name, entry->type);
    if (existing != NULL) {
        return -2;  
    }
    
    entry->inode = vfs_allocate_inode();
    if (entry->inode == (unsigned int)-1) {
        return -3;
    }
    
    vfs_memcpy(&vfs.entries[entry->inode], entry, sizeof(vfs_entry));
    
    vfs.entries[entry->parent].size += sizeof(vfs_entry);
    
    return 0;
}

static int vfs_remove_entry(unsigned int inode) {
    if (inode == 0) return -1;  
    if (inode >= vfs.entry_count) return -2;
    
    if (vfs.entries[inode].type == VFS_TYPE_DIRECTORY) {
        for (unsigned int i = 0; i < vfs.entry_count; i++) {
            if (vfs.entries[i].parent == inode) {
                return -3;  
            }
        }
    }

    if (vfs.entries[inode].type == VFS_TYPE_FILE) {
        vfs_memset(data_blocks[vfs.entries[inode].data_block], 0, VFS_MAX_CONTENT);
    }

    vfs_memset(&vfs.entries[inode], 0, sizeof(vfs_entry));
    
    return 0;
}

int vfs_mkdir(const char *name) {
    unsigned int name_len = strlen(name);
    if (name_len >= VFS_MAX_NAME) {
        printk(RED, "ERR: Name too long");
        return -1;
    }
    
    vfs_entry new_dir;
    vfs_memset(&new_dir, 0, sizeof(vfs_entry));
    
    strcpy(new_dir.name, name);
    new_dir.type = VFS_TYPE_DIRECTORY;
    new_dir.parent = vfs.current_directory;
    new_dir.size = 0;
    new_dir.data_block = 0;
    new_dir.created_time = 0;  
    new_dir.modified_time = 0;
    
    int result = vfs_add_entry(&new_dir);
    if (result < 0) {
        if (result == -2) {
            printk(RED, "ERR: Directory already exists");
        } else {
            printk(RED, "ERR: Cannot create directory");
        }
        return -1;
    }
    
    printk(WHITE, "VFS: Directory created\n");

    return 0;
}

void vfs_create_sys_node(const char *path, int (*read_fn)(char *)) {

    printk(WHITE, "\nVFS: Registered sys-node: %s\n", path);

}

int vfs_create(const char *name, const char *content) {
    unsigned int name_len = strlen(name);
    if (name_len >= VFS_MAX_NAME) {
        printk(RED, "ERR: Name too long");
        return -1;
    }
    
    unsigned int content_len = strlen(content);
    if (content_len >= VFS_MAX_CONTENT) {
        printk(RED, "ERR: Content too large");
        return -2;
    }
    
    vfs_entry new_file;
    vfs_memset(&new_file, 0, sizeof(vfs_entry));
    
    strcpy(new_file.name, name);
    new_file.type = VFS_TYPE_FILE;
    new_file.parent = vfs.current_directory;
    new_file.size = content_len;
    new_file.data_block = new_file.inode;  
    new_file.created_time = 0;
    new_file.modified_time = 0;
    
    vfs_memcpy(data_blocks[new_file.data_block], content, content_len);
    data_blocks[new_file.data_block][content_len] = '\0';
    
    int result = vfs_add_entry(&new_file);
    if (result < 0) {
        if (result == -2) {
            printk(RED, "ERR: File already exists");
        } else {
            printk(RED, "ERR: Cannot create file");
        }
        return -1;
    }
    
    printk(GREEN, "File created");
    return 0;
}

char* vfs_read(const char *name) {
    // 1. Obtener la ruta completa (pwd + name)
    char full_path[VFS_MAX_PATH];
    strcpy(full_path, vfs_pwd());
    if (full_path[strlen(full_path)-1] != '/') vfs_strcat(full_path, "/");
    vfs_strcat(full_path, name);

    // 2. ¿Esta ruta empieza con algún punto de montaje?
    for (int i = 0; i < MAX_MOUNTS; i++) {
        if (mount_table[i].active && strncmp(full_path, mount_table[i].target, strlen(mount_table[i].target)) == 0) {
            
            if (strcmp(mount_table[i].type, "9p") == 0) {
                // LLAMAR AL DRIVER DE 9P REAL
                return v9p_driver_read(full_path); 
            }
            if (strcmp(mount_table[i].type, "adfs") == 0) {
                // LLAMAR AL DRIVER DE ADFS
                return adfs_driver_read(full_path);
            }
        }
    }

    // 3. Si no es un montaje, usar tu lógica de RAM actual
    vfs_entry *file = vfs_find_in_directory(vfs.current_directory, name, VFS_TYPE_FILE);
    if (file == NULL) return NULL;
    
    vfs_memcpy(read_buffer, data_blocks[file->data_block], file->size);
    read_buffer[file->size] = '\0';
    return read_buffer;
}

int vfs_write(const char *name, const char *content) {
    vfs_entry *file = vfs_find_in_directory(vfs.current_directory, name, VFS_TYPE_FILE);
    if (file == NULL) {
        return vfs_create(name, content);
    }
    
    unsigned int content_len = strlen(content);
    if (content_len >= VFS_MAX_CONTENT) {
        printk(RED, "ERR: Content too large");
        return -1;
    }
    
    vfs_memcpy(data_blocks[file->data_block], content, content_len);
    file->size = content_len;
    data_blocks[file->data_block][content_len] = '\0';
    
    printk(GREEN, "File written");
    return 0;
}

void vfs_register_node(const char *path, int is_dir, int (*callback)(char *)) {
    if (total_vfs_nodes >= MAX_VFS_NODES) return;

    strncpy(vfs_nodes[total_vfs_nodes].path, path, 254);
    vfs_nodes[total_vfs_nodes].is_directory = is_dir;
    vfs_nodes[total_vfs_nodes].read_callback = callback;
    strcpy(directory_table[directory_count].name, "sys");
    directory_table[directory_count].is_vfs = 1;
    directory_table[directory_count].parent_dir = 0;
    total_vfs_nodes++;
}
void vfs_list_files_in_dir(const char *dir_path) {
    int dir_len = strlen(dir_path);
    
    for (int i = 0; i < total_vfs_nodes; i++) {
        /* Check if the node path starts with the current directory */
        if (strncmp(vfs_nodes[i].path, dir_path, dir_len) == 0) {
            
            /* Pointer to the name after the directory prefix */
            char *name = vfs_nodes[i].path + dir_len;

            /* If it's the root directory or starts with slash, skip the slash */
            if (*name == '/') name++;

            /* Only list items in the immediate directory (no deeper slashes) */
            if (strlen(name) > 0 && strchr(name, '/') == 0) {
                if (vfs_nodes[i].is_directory) {
                    printk(CYAN, "%s/  ", name);
                } else {
                    printk(WHITE, "%s  ", name);
                }
            }
        }
    }
    printk(WHITE, "\n");
}

int vfs_delete(const char *name) {
    vfs_entry *entry = vfs_find_in_directory(vfs.current_directory, name, VFS_TYPE_FILE);
    if (entry == NULL) {
        entry = vfs_find_in_directory(vfs.current_directory, name, VFS_TYPE_DIRECTORY);
    }
    
    if (entry == NULL) {
        printk(RED, "ERR: Entry not found");
        return -1;
    }
    
    int result = vfs_remove_entry(entry->inode);
    if (result < 0) {
        if (result == -3) {
            printk(RED, "ERR: Directory not empty");
        } else {
            printk(RED, "ERR: Cannot delete");
        }
        return -1;
    }
    
    printk(GREEN, "Entry deleted");
    return 0;
}

void vfs_ls(void) {
    vfs_system* sys = get_vfs_instance();
    unsigned int count = 0;
    char* current_p = vfs_pwd(); // Obtenemos la ruta actual (ej: "/mnt")

    printk(CYAN, "  .  \n  .. ");

    // --- NUEVO: Mostrar puntos de montaje ---
    for (int i = 0; i < MAX_MOUNTS; i++) {
        if (mount_table[i].active) {
            // Si estamos en la raíz "/" y el montaje es "/mnt", 
            // queremos que aparezca "mnt/" en la lista.
            
            // Lógica simple: si el target del montaje empieza con el path actual
            // pero es un nivel más profundo, lo mostramos.
            if (strncmp(mount_table[i].target, current_p, strlen(current_p)) == 0) {
                // Evitamos mostrar el "." si el target es igual al actual
                if (strcmp(mount_table[i].target, current_p) != 0) {
                    printk(BLUE, "\n  %s/ (MOUNT: %s)", mount_table[i].target, mount_table[i].type);
                    count++;
                }
            }
        }
    }

    // --- Lógica original: Mostrar entradas de la RAM ---
    for (unsigned int i = 0; i < sys->entry_count; i++) {
        if (sys->entries[i].parent == sys->current_directory) {
            if (sys->entries[i].type == VFS_TYPE_DIRECTORY) {
                printk(BLUE, "\n  %s/", sys->entries[i].name);
            } else {
                printk(GRAY, "\n  %s", sys->entries[i].name);
            }
            count++;
        }
    }

    if (count == 0) {
        printk(RED, "\n  Empty directory\n");
    }
    printk(WHITE, "\n");
}

int vfs_cd(const char *path) {
    if (strcmp(path, "..") == 0) {
        if (vfs.current_directory != vfs.root_directory) {
            vfs.current_directory = vfs.entries[vfs.current_directory].parent;
        }
        return 0;
    }
    
    if (strcmp(path, ".") == 0 || strcmp(path, "/") == 0) {
        if (strcmp(path, "/") == 0) {
            vfs.current_directory = vfs.root_directory;
        }
        return 0;
    }
    

    vfs_entry *dir = vfs_find_in_directory(vfs.current_directory, path, VFS_TYPE_DIRECTORY);
    if (dir == NULL) {
        printk(RED, "ERR: Directory not found");
        return -1;
    }
    
    vfs.current_directory = dir->inode;
    return 0;
}

char* vfs_pwd(void) {
    static char path[VFS_MAX_PATH];
    char temp[VFS_MAX_PATH];
    
    vfs_memset(path, 0, VFS_MAX_PATH);
    vfs_memset(temp, 0, VFS_MAX_PATH);
    
    unsigned int current = vfs.current_directory;
    

    if (current == vfs.root_directory) {
        strcpy(path, "/");
        return path;
    }
    

    while (current != vfs.root_directory) {
        temp[0] = '/';
        temp[1] = '\0';
        vfs_strcat(temp, vfs.entries[current].name);
        vfs_strcat(temp, path);
        
        strcpy(path, temp);
        

        temp[0] = '\0';
        
        current = vfs.entries[current].parent;
    }
    

    if (strlen(path) == 0) {
        strcpy(path, "/");
    }
    
    return path;
}

int vfs_exists(const char *path) {
    vfs_entry *entry = vfs_find_in_directory(vfs.current_directory, path, VFS_TYPE_FILE);
    if (entry != NULL) return 1;
    
    entry = vfs_find_in_directory(vfs.current_directory, path, VFS_TYPE_DIRECTORY);
    if (entry != NULL) return 1;
    
    return 0;
}

vfs_entry* vfs_find_entry(const char *name, vfs_entry_type type) {
    return vfs_find_in_directory(vfs.current_directory, name, type);
}

vfs_entry* vfs_get_entry_by_inode(unsigned int inode) {
    if (inode >= vfs.entry_count) {
        return NULL;
    }
    return &vfs.entries[inode];
}


int vfs_open(const char *name, unsigned int mode) {
    vfs_entry *file = vfs_find_in_directory(vfs.current_directory, name, VFS_TYPE_FILE);
    if (file == NULL) {
        return -1;
    }
    
    for (int i = 0; i < VFS_MAX_OPEN_FILES; i++) {
        if (open_files[i].ref_count == 0) {
            open_files[i].inode = file->inode;
            open_files[i].position = 0;
            open_files[i].mode = mode;
            open_files[i].ref_count = 1;
            return i;  
        }
    }
    
    return -2; 
}

int vfs_close(int fd) {
    if (fd < 0 || fd >= VFS_MAX_OPEN_FILES) {
        return -1;
    }
    
    if (open_files[fd].ref_count == 0) {
        return -2;  
    }
    
    open_files[fd].ref_count = 0;
    return 0;
}

int vfs_read_fd(int fd, char *buffer, unsigned int size) {
    if (fd < 0 || fd >= VFS_MAX_OPEN_FILES || open_files[fd].ref_count == 0) {
        return -1;
    }
    
    vfs_entry *file = vfs_get_entry_by_inode(open_files[fd].inode);
    if (file == NULL || file->type != VFS_TYPE_FILE) {
        return -2;
    }
    
    unsigned int remaining = file->size - open_files[fd].position;
    unsigned int to_read = (size < remaining) ? size : remaining;
    
    if (to_read > 0) {
        vfs_memcpy(buffer, &data_blocks[file->data_block][open_files[fd].position], to_read);
        open_files[fd].position += to_read;
    }
    
    return to_read;
}

int vfs_write_fd(int fd, const char *buffer, unsigned int size) {
    if (fd < 0 || fd >= VFS_MAX_OPEN_FILES || open_files[fd].ref_count == 0) {
        return -1;
    }
    
    if (!(open_files[fd].mode & 2)) { 
        return -3;  
    }
    
    vfs_entry *file = vfs_get_entry_by_inode(open_files[fd].inode);
    if (file == NULL || file->type != VFS_TYPE_FILE) {
        return -2;
    }
    
    if (open_files[fd].position + size >= VFS_MAX_CONTENT) {
        size = VFS_MAX_CONTENT - open_files[fd].position - 1;
    }
    
    if (size > 0) {
        vfs_memcpy(&data_blocks[file->data_block][open_files[fd].position], buffer, size);
        open_files[fd].position += size;
        
        if (open_files[fd].position > file->size) {
            file->size = open_files[fd].position;
        }
    }
    
    return size;
}
void vfs_cat(const char *name) {
    vfs_system* sys = get_vfs_instance();
    int found = 0;

    for (unsigned int i = 0; i < sys->entry_count; i++) { 
        if (strcmp(sys->entries[i].name, name) == 0 && 
            sys->entries[i].parent == sys->current_directory) {
        
            if (sys->entries[i].type == VFS_TYPE_DIRECTORY) {
                printk(RED, "cat: %s: Is a directory\n", name);
                return;
            }

            printk(WHITE, "%s\n", data_blocks[sys->entries[i].data_block]);
            found = 1;
            break;
        }
    }

    if (!found) {
        printk(RED, "cat: %s: No such file\n", name);
    }
}



void vfs_rm(const char *name) {
    vfs_system* sys = get_vfs_instance();
    
    for (unsigned int i = 0; i < sys->entry_count; i++) {
        if (strcmp(sys->entries[i].name, name) == 0 && 
            sys->entries[i].parent == sys->current_directory) {
            
            if (sys->entries[i].type == VFS_TYPE_DIRECTORY) {
                printk(RED, "\nrm: %s is a directory. Use rmdir.\n", name);
                return;
            }

           
            vfs_memset(data_blocks[sys->entries[i].data_block], 0, VFS_MAX_CONTENT);
            
            for (unsigned int j = i; j < sys->entry_count - 1; j++) {
                sys->entries[j] = sys->entries[j + 1];
            }
            sys->entry_count--;
            
            printk(GREEN, "\nFile '%s' removed.\n", name);
            return;
        }
    }
    printk(RED, "\nrm: %s not found.\n", name);
}

void vfs_rmdir(const char *name) {
    vfs_system* sys = get_vfs_instance();
    
    for (unsigned int i = 0; i < sys->entry_count; i++) {
        if (strcmp(sys->entries[i].name, name) == 0 && 
            sys->entries[i].parent == sys->current_directory) {
            
            if (sys->entries[i].type != VFS_TYPE_DIRECTORY) {
                printk(RED, "\nrmdir: %s is not a directory.\n", name);
                return;
            }

            for (unsigned int j = i; j < sys->entry_count - 1; j++) {
                sys->entries[j] = sys->entries[j + 1];
            }
            sys->entry_count--;
            
            printk(GREEN, "\nDirectory '%s' removed.\n", name);
            return;
        }
    }
    printk(RED, "\nrmdir: %s not found.\n", name);
}

int vfs_mount(const char *source, const char *target, const char *type) {
    for (int i = 0; i < MAX_MOUNTS; i++) {
        if (!mount_table[i].active) {
            strcpy(mount_table[i].target, target);
            strcpy(mount_table[i].type, type);
            mount_table[i].active = 1;

            pr_info("VFS: %s mounted on %s as %s\n", source, target, type);
            return 0;
        }
    }
    return -1;
}

vfs_mount_point mount_table[MAX_MOUNTS];