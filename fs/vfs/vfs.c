#include <fs/vfs.h>
#include <fs/fs.h>
#include <kernel/colors.h>
#include <kernel/printk.h>
#include <lib/string.h>

/* --- Global Tables --- */
vfs_node_t vfs_nodes[MAX_VFS_NODES];
int total_vfs_nodes = 0;
vfs_mount_point mount_table[MAX_MOUNTS];

static vfs_system vfs;
static vfs_file_handle open_files[VFS_MAX_OPEN_FILES];
static char read_buffer[VFS_MAX_CONTENT];

/* RAM-Disk Storage: The heart of our temporary FS */
static char data_blocks[VFS_MAX_ENTRIES][VFS_MAX_CONTENT];

/* --- Internal Helpers --- */

vfs_system* get_vfs_instance(void) {
    return &vfs;
}

/**
 * vfs_find_in_directory: Searches for an entry within a specific directory.
 */
static vfs_entry* vfs_find_in_directory(unsigned int dir_inode, const char *name, vfs_entry_type type) {
    for (unsigned int i = 0; i < vfs.entry_count; i++) {
        if (vfs.entries[i].parent == dir_inode && 
            vfs.entries[i].type == type &&
            strcmp(vfs.entries[i].name, name) == 0) {
            return &vfs.entries[i];
        }
    }
    return NULL;
}

/* --- Core VFS Functions --- */

/**
 * vfs_init: Resets all tables and creates the root directory.
 */
void vfs_init(void) {
    memset(&vfs, 0, sizeof(vfs));
    memset(open_files, 0, sizeof(open_files));
    memset(data_blocks, 0, sizeof(data_blocks));
    memset(mount_table, 0, sizeof(mount_table));
    
    /* Initialize Root Directory "/" */
    vfs.entries[0].inode = 0;
    strcpy(vfs.entries[0].name, "/");
    vfs.entries[0].type = VFS_TYPE_DIRECTORY;
    vfs.entries[0].parent = 0;  
    vfs.entries[0].size = 0;
    
    vfs.entry_count = 1;
    vfs.current_directory = 0;
    vfs.root_directory = 0;

    printk(CYAN, "VFS: Virtual File System initialized successfully.\n");
}

fs_initcall(vfs_init);

/**
 * vfs_mkdir: Creates a new directory in the current path.
 */
int vfs_mkdir(const char *name) {
    if (vfs.entry_count >= VFS_MAX_ENTRIES) return -1;
    if (vfs_find_in_directory(vfs.current_directory, name, VFS_TYPE_DIRECTORY)) return -2;

    vfs_entry *new_dir = &vfs.entries[vfs.entry_count];
    memset(new_dir, 0, sizeof(vfs_entry));
    
    strncpy(new_dir->name, name, VFS_MAX_NAME - 1);
    new_dir->type = VFS_TYPE_DIRECTORY;
    new_dir->parent = vfs.current_directory;
    new_dir->inode = vfs.entry_count;
    
    vfs.entry_count++;
    return 0;
}

/**
 * vfs_create: Creates a new file with content.
 */
int vfs_create(const char *name, const char *content) {
    if (vfs.entry_count >= VFS_MAX_ENTRIES) return -1;
    
    unsigned int content_len = strlen(content);
    if (content_len >= VFS_MAX_CONTENT) return -2;
    
    vfs_entry *new_file = &vfs.entries[vfs.entry_count];
    memset(new_file, 0, sizeof(vfs_entry));
    
    strncpy(new_file->name, name, VFS_MAX_NAME - 1);
    new_file->type = VFS_TYPE_FILE;
    new_file->parent = vfs.current_directory;
    new_file->inode = vfs.entry_count;
    new_file->size = content_len;
    new_file->data_block = vfs.entry_count;
    
    memcpy(data_blocks[new_file->data_block], content, content_len);
    data_blocks[new_file->data_block][content_len] = '\0';
    
    vfs.entry_count++;
    return 0;
}

/* --- Shell-like Commands --- */

void vfs_ls(void) {
    unsigned int count = 0;
    char* pwd = vfs_pwd();

    printk(CYAN, ".  ..  ");

    /* 1. List Mount Points */
    for (int i = 0; i < MAX_MOUNTS; i++) {
        if (mount_table[i].active) {
            /* Basic logic: show if the mount is under current directory */
            if (strncmp(mount_table[i].target, pwd, strlen(pwd)) == 0) {
                 printk(BLUE, "%s/ (mount)  ", mount_table[i].target);
                 count++;
            }
        }
    }

    /* 2. List RAM entries */
    for (unsigned int i = 0; i < vfs.entry_count; i++) {
        if (vfs.entries[i].parent == vfs.current_directory && i != 0) {
            if (vfs.entries[i].type == VFS_TYPE_DIRECTORY)
                printk(BLUE, "%s/  ", vfs.entries[i].name);
            else
                printk(WHITE, "%s  ", vfs.entries[i].name);
            count++;
        }
    }
    printk(WHITE, "\n");
}

int vfs_cd(const char *path) {
    if (strcmp(path, "..") == 0) {
        vfs.current_directory = vfs.entries[vfs.current_directory].parent;
        return 0;
    }
    if (strcmp(path, "/") == 0) {
        vfs.current_directory = vfs.root_directory;
        return 0;
    }

    vfs_entry *dir = vfs_find_in_directory(vfs.current_directory, path, VFS_TYPE_DIRECTORY);
    if (dir) {
        vfs.current_directory = dir->inode;
        return 0;
    }
    
    printk(RED, "cd: %s: No such directory\n", path);
    return -1;
}

/**
 * vfs_pwd: Reconstructs the absolute path.
 */
char* vfs_pwd(void) {
    static char path[VFS_MAX_PATH];
    char temp[VFS_MAX_PATH];
    unsigned int current = vfs.current_directory;

    if (current == vfs.root_directory) return "/";

    memset(path, 0, VFS_MAX_PATH);
    while (current != vfs.root_directory) {
        memset(temp, 0, VFS_MAX_PATH);
        strcpy(temp, "/");
        strcat(temp, vfs.entries[current].name);
        strcat(temp, path);
        strcpy(path, temp);
        current = vfs.entries[current].parent;
    }
    return path;
}

void vfs_cat(const char *name) {
    vfs_entry *file = vfs_find_in_directory(vfs.current_directory, name, VFS_TYPE_FILE);
    if (file) {
        printk(WHITE, "%s\n", data_blocks[file->data_block]);
    } else {
        printk(RED, "cat: %s: No such file\n", name);
    }
}

/* --- Mounting System --- */

int vfs_mount(const char *source, const char *target, const char *type) {
    for (int i = 0; i < MAX_MOUNTS; i++) {
        if (!mount_table[i].active) {
            strncpy(mount_table[i].target, target, VFS_MAX_PATH - 1);
            strncpy(mount_table[i].type, type, 15);
            mount_table[i].active = 1;
            return 0;
        }
    }
    return -1;
}