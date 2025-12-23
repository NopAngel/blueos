#include "../include/fs/vfs.h"

extern int cursor_y;

static vfs_system vfs;
static vfs_file_handle open_files[VFS_MAX_OPEN_FILES];
static char current_path[VFS_MAX_PATH] = "/";

static char read_buffer[VFS_MAX_CONTENT];

static char data_blocks[VFS_MAX_ENTRIES][VFS_MAX_CONTENT];


static void vfs_memset(void *ptr, char value, unsigned int size) {
    char *p = (char *)ptr;
    for (unsigned int i = 0; i < size; i++) {
        p[i] = value;
    }
}

static void vfs_memcpy(void *dest, const void *src, unsigned int size) {
    char *d = (char *)dest;
    const char *s = (const char *)src;
    for (unsigned int i = 0; i < size; i++) {
        d[i] = s[i];
    }
}

static void vfs_strcat(char *dest, const char *src) {
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
    
    printk("VFS initialized", cursor_y++, GREEN);
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
        printk("ERR: Name too long", cursor_y++, RED);
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
            printk("ERR: Directory already exists", cursor_y++, RED);
        } else {
            printk("ERR: Cannot create directory", cursor_y++, RED);
        }
        return -1;
    }
    
    printk("Directory created", cursor_y++, GREEN);
    return 0;
}

int vfs_create(const char *name, const char *content) {
    unsigned int name_len = strlen(name);
    if (name_len >= VFS_MAX_NAME) {
        printk("ERR: Name too long", cursor_y++, RED);
        return -1;
    }
    
    unsigned int content_len = strlen(content);
    if (content_len >= VFS_MAX_CONTENT) {
        printk("ERR: Content too large", cursor_y++, RED);
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
            printk("ERR: File already exists", cursor_y++, RED);
        } else {
            printk("ERR: Cannot create file", cursor_y++, RED);
        }
        return -1;
    }
    
    printk("File created", cursor_y++, GREEN);
    return 0;
}

char* vfs_read(const char *name) {
    vfs_entry *file = vfs_find_in_directory(vfs.current_directory, name, VFS_TYPE_FILE);
    if (file == NULL) {
        printk("ERR: File not found", cursor_y++, RED);
        return NULL;
    }
    
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
        printk("ERR: Content too large", cursor_y++, RED);
        return -1;
    }
    
    vfs_memcpy(data_blocks[file->data_block], content, content_len);
    file->size = content_len;
    data_blocks[file->data_block][content_len] = '\0';
    
    printk("File written", cursor_y++, GREEN);
    return 0;
}

int vfs_delete(const char *name) {
    vfs_entry *entry = vfs_find_in_directory(vfs.current_directory, name, VFS_TYPE_FILE);
    if (entry == NULL) {
        entry = vfs_find_in_directory(vfs.current_directory, name, VFS_TYPE_DIRECTORY);
    }
    
    if (entry == NULL) {
        printk("ERR: Entry not found", cursor_y++, RED);
        return -1;
    }
    
    int result = vfs_remove_entry(entry->inode);
    if (result < 0) {
        if (result == -3) {
            printk("ERR: Directory not empty", cursor_y++, RED);
        } else {
            printk("ERR: Cannot delete", cursor_y++, RED);
        }
        return -1;
    }
    
    printk("Entry deleted", cursor_y++, GREEN);
    return 0;
}

void vfs_ls(void) {
    cursor_y++;
    printk(".", cursor_y++, BLUE);
    printk("..", cursor_y++, BLUE);
    
    unsigned int count = 0;
    
    for (unsigned int i = 0; i < vfs.entry_count; i++) {
        if (vfs.entries[i].parent == vfs.current_directory) {
            if (vfs.entries[i].type == VFS_TYPE_DIRECTORY) {
                printk(vfs.entries[i].name, cursor_y++, BLUE);
            } else {
                printk(vfs.entries[i].name, cursor_y++, GRAY);
            }
            count++;
        }
    }
    
    if (count == 0) {
        printk("Empty directory", cursor_y++, RED);
    }
    
    cursor_y++;
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
        printk("ERR: Directory not found", cursor_y++, RED);
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