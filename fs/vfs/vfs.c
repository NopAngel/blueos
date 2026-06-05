#include <fs/vfs.h>
#include <lib/string.h>
#include <kernel/malloc.h>
#include <kernel/printk.h>
#include <kernel/colors.h>
#include <mm/memory.h>

/* --- Global VFS pointers --- */
static vfs_node_t* vfs_root = NULL;
static vfs_node_t* vfs_current = NULL;

static char* strrchr_local(const char* str, int c) {
    char* last = NULL;
    for (const char* p = str; *p; p++) {
        if (*p == c) {
            last = (char*)p;
        }
    }
    return last;
}

vfs_node_t* vfs_lookup(const char* path) {
    if (!path) return NULL;
    if (strcmp(path, "/") == 0) return vfs_root;

    vfs_node_t* curr = (path[0] == '/') ? vfs_root : vfs_current;

    char tmp_path[256];
    strncpy(tmp_path, path, 255);

    char* part = strtok(tmp_path, "/");
    while (part != NULL) {
        if (strcmp(part, ".") == 0) {
            part = strtok(NULL, "/");
            continue;
        }
        if (strcmp(part, "..") == 0) {
            if (curr->parent) curr = curr->parent;
            part = strtok(NULL, "/");
            continue;
        }
        if (curr->ops && curr->ops->finddir) {
            curr = curr->ops->finddir(curr, part);
            if (!curr) return NULL;
        } else {
            return NULL;
        }
        part = strtok(NULL, "/");
    }
    return curr;
}

/* Funciones auxiliares para el manejo de nodos en memoria */
static struct vfs_node* root_finddir(struct vfs_node* node, const char* name) {
    vfs_node_t* curr = node->ptr; // Usamos ptr como cabeza de la lista de hijos
    while (curr) {
        if (strcmp(curr->name, name) == 0) return curr;
        curr = curr->next;
    }
    return NULL;
}

static int root_read(struct vfs_node* node, void* buffer, uint32_t size, uint32_t offset) {
    if (!node->ptr) return -1;
    if (offset >= node->size) return 0;
    if (offset + size > node->size) size = node->size - offset;
    memcpy(buffer, (char*)node->ptr + offset, size);
    return size;
}

static int root_mkdir(struct vfs_node* node, const char* name, uint16_t mode) {
    vfs_node_t* child = (vfs_node_t*)kmalloc(sizeof(vfs_node_t));
    if (!child) return -1;
    mm_memset(child, 0, sizeof(vfs_node_t));
    
    strncpy(child->name, name, VFS_NAME_MAX);
    child->type = VFS_TYPE_DIR;
    child->ops = node->ops;
    child->parent = node;
    
    // Insertar en la lista de hijos
    child->next = node->ptr;
    node->ptr = child;
    return 0;
}

static int root_create(struct vfs_node* node, const char* name, const char* content) {
    vfs_node_t* child = (vfs_node_t*)kmalloc(sizeof(vfs_node_t));
    if (!child) return -1;
    mm_memset(child, 0, sizeof(vfs_node_t));
    
    strncpy(child->name, name, VFS_NAME_MAX);
    child->type = VFS_TYPE_FILE;
    child->size = (content == NULL) ? 0 : strlen(content);
    child->ptr = (void*)content;
    child->ops = node->ops;
    child->parent = node;
    
    // Insertar en la lista de hijos
    child->next = node->ptr;
    node->ptr = child;
    return 0;
}

/**
 * root_readdir - Implementation for the root directory listing
 */
int root_readdir(struct vfs_node* node, uint32_t index, void* dirent_out) {
    struct vfs_dirent* de = (struct vfs_dirent*)dirent_out;
    vfs_node_t* curr = node->ptr;
    uint32_t i = 0;
    
    while (curr && i < index) {
        curr = curr->next;
        i++;
    }

    if (curr) {
        strncpy(de->name, curr->name, VFS_NAME_MAX);
        de->type = (uint32_t)curr->type;
        de->inode = curr->inode;
        return 0;
    }
    return -1;
}

static int remove_node(vfs_node_t* node) {
    if (!node) return -1;

    if (node->type == VFS_TYPE_DIR) {
        vfs_node_t* child = node->ptr;
        while (child) {
            vfs_node_t* next = child->next;
            if (remove_node(child) != 0) return -1;
            child = next;
        }
    }

    kfree(node);
    return 0;
}

static int root_unlink(struct vfs_node* node, const char* name) {
    if (!node || !name) return -1;

    vfs_node_t* curr = node->ptr;
    vfs_node_t* prev = NULL;

    while (curr) {
        if (strcmp(curr->name, name) == 0) break;
        prev = curr;
        curr = curr->next;
    }

    if (!curr) return -1;
    if (curr == vfs_root) return -1;

    vfs_node_t* next = curr->next;
    if (prev) {
        prev->next = next;
    } else {
        node->ptr = next;
    }

    return remove_node(curr);
}

vfs_ops_t root_ops = {
    .readdir = (void*)root_readdir,
    .finddir = (void*)root_finddir,
    .read    = (void*)root_read,
    .write   = NULL,
    .open    = NULL,
    .close   = NULL,
    .mkdir   = (void*)root_mkdir,
    .create  = (void*)root_create,
    .unlink  = (void*)root_unlink
};

/* --- Core VFS Functions --- */

int vfs_open(vfs_node_t* node, uint32_t flags) {
    if (node && node->ops && node->ops->open) {
        return node->ops->open(node, flags);
    }
    return 0;
}

int vfs_close(vfs_node_t* node) {
    if (node && node->ops && node->ops->close) {
        return node->ops->close(node);
    }
    return 0;
}

int vfs_read(vfs_node_t* node, void* buffer, uint32_t size, uint32_t offset) {
    if (node && node->ops && node->ops->read) {
        return node->ops->read(node, buffer, size, offset);
    }
    return -1;
}

int vfs_write(vfs_node_t* node, const void* buffer, uint32_t size, uint32_t offset) {
    if (node && node->ops && node->ops->write) {
        return node->ops->write(node, buffer, size, offset);
    }
    return -1;
}

/**
 * vfs_readdir - Fixed with generic pointer passing
 */
int vfs_readdir(vfs_node_t* node, uint32_t index, struct vfs_dirent* dirent) {
    if (node && (node->type == VFS_TYPE_DIR) && node->ops && node->ops->readdir) {
        // We cast the function pointer call to handle the dirent as a generic pointer
        int (*readdir_func)(struct vfs_node*, uint32_t, void*) = (void*)node->ops->readdir;
        return readdir_func((struct vfs_node*)node, index, (void*)dirent);
    }
    return -1;
}

vfs_node_t* vfs_finddir(vfs_node_t* node, const char* name) {
    if (node && (node->type == VFS_TYPE_DIR) && node->ops && node->ops->finddir) {
        return (vfs_node_t*)node->ops->finddir((struct vfs_node*)node, name);
    }
    return NULL;
}

/* --- Navigation & Init --- */

void vfs_init(void) {
    vfs_root = (vfs_node_t*)kmalloc(sizeof(vfs_node_t));
    if (!vfs_root) return;

    mm_memset(vfs_root, 0, sizeof(vfs_node_t));
    strcpy(vfs_root->name, "/");
    vfs_root->type = VFS_TYPE_DIR;
    vfs_root->ops = &root_ops;

    vfs_current = vfs_root;
    boot_msg("VFS", "Successfully initialized root filesystem.\n", 0);
}

vfs_node_t* vfs_get_root(void) {
    return vfs_root;
}

vfs_node_t* vfs_get_current(void) {
    return vfs_current;
}

int vfs_chdir(const char* path) {
    vfs_node_t* node = vfs_lookup(path);
    if (node && node->type == VFS_TYPE_DIR) {
        vfs_current = node;
        return 0;
    }
    return -1;
}

void vfs_get_cwd(char* buffer, uint32_t size) {
    vfs_node_t* curr = vfs_get_current();
    if (!curr) {
        strncpy(buffer, "/", size);
        return;
    }

    if (curr == vfs_root) {
        strncpy(buffer, "/", size);
        return;
    }

    const char* parts[32];
    int depth = 0;
    while (curr && curr != vfs_root && depth < 32) {
        parts[depth++] = curr->name;
        curr = curr->parent;
    }

    int pos = 0;
    if (pos < (int)size) {
        buffer[pos++] = '/';
    }

    for (int i = depth - 1; i >= 0; i--) {
        int len = strlen(parts[i]);
        if (pos + len + 1 >= (int)size) break;
        memcpy(buffer + pos, parts[i], len);
        pos += len;
        if (i > 0) {
            buffer[pos++] = '/';
        }
    }
    if (pos < (int)size) buffer[pos] = '\0';
    else buffer[size - 1] = '\0';
}

int vfs_mount(const char* dev, const char* target) {
    vfs_node_t* mount_point = vfs_lookup(target);
    if (!mount_point) {
        if (vfs_mkdir(target) != 0) return -1;
        mount_point = vfs_lookup(target);
        if (!mount_point) return -1;
    }

    /* Verificar carpetas del sistema esenciales */
    const char* system_dirs[] = {
        "/rescue", "/sys", "/dev", "/bin", "/usr", "/etc", "/conf", "/kernel",
        "/tmp", "/mnt", "/boot", "/home", "/lib", "/media", "/opt",
        "/proc", "/run", "/sbin", "/srv", "/var", "/root"
    };
    for (int i = 0; i < (int)(sizeof(system_dirs) / sizeof(system_dirs[0])); i++) {
        vfs_node_t* check = vfs_lookup(system_dirs[i]);
        if (!check) {
            boot_msg("VFS", "System directory not found. Creating...\n", 1);
            if (vfs_mkdir(system_dirs[i]) != 0) {
                boot_msg("VFS", "Failed to create\n", 2);
            }
        }
    }

    boot_msg("VFS", "Mounted device", 0);
    return 0;
}

int vfs_create_binary(const char* name, void* buffer, uint32_t size) {
    vfs_node_t* curr = vfs_get_current();
    if (!curr) return -1;

    vfs_node_t* child = (vfs_node_t*)kmalloc(sizeof(vfs_node_t));
    if (!child) return -1;
    mm_memset(child, 0, sizeof(vfs_node_t));
    
    strncpy(child->name, name, VFS_NAME_MAX);
    child->type = VFS_TYPE_FILE;
    child->size = size;
    child->ptr = buffer;
    child->ops = &root_ops;
    child->parent = curr;
    
    child->next = curr->ptr;
    curr->ptr = child;
    return 0;
}

static vfs_node_t* vfs_parent_for_path(const char* path, char* name_buf, uint32_t buf_size) {
    if (!path || !name_buf || buf_size == 0) return NULL;

    char tmp[256];
    strncpy(tmp, path, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';

    char* last_slash = strrchr_local(tmp, '/');
    if (!last_slash) {
        strncpy(name_buf, tmp, buf_size);
        name_buf[buf_size - 1] = '\0';
        return vfs_get_current();
    }

    if (last_slash == tmp) {
        strncpy(name_buf, last_slash + 1, buf_size);
        name_buf[buf_size - 1] = '\0';
        return vfs_root;
    }

    *last_slash = '\0';
    strncpy(name_buf, last_slash + 1, buf_size);
    name_buf[buf_size - 1] = '\0';
    return vfs_lookup(tmp);
}

int vfs_mkdir(const char* path) {
    if (!path || path[0] == '\0') return -1;

    char name[VFS_NAME_MAX];
    vfs_node_t* parent = vfs_parent_for_path(path, name, sizeof(name));
    if (!parent || parent->type != VFS_TYPE_DIR || !parent->ops || !parent->ops->mkdir) return -1;

    return parent->ops->mkdir(parent, name, 0777);
}

int vfs_create(const char* path, const char* content) {
    if (!path || path[0] == '\0') return -1;

    char name[VFS_NAME_MAX];
    vfs_node_t* parent = vfs_parent_for_path(path, name, sizeof(name));
    if (!parent || parent->type != VFS_TYPE_DIR || !parent->ops || !parent->ops->create) return -1;

    return parent->ops->create(parent, name, content);
}

int vfs_touch(const char* path, const char* content) {
    return vfs_create(path, content);
}

int vfs_unlink(const char* path) {
    if (!path || strlen(path) == 0) return -1;

    char name[VFS_NAME_MAX];
    vfs_node_t* parent = vfs_parent_for_path(path, name, sizeof(name));
    if (!parent || parent->type != VFS_TYPE_DIR || !parent->ops || !parent->ops->unlink) return -1;

    return parent->ops->unlink(parent, name);
}

int vfs_rm(const char* path) {
    if (!path || strcmp(path, "/") == 0) return -1; 
    return vfs_unlink(path);
}

vfs_node_t* vfs_findfile(const char* path) {
    vfs_node_t* node = vfs_lookup(path);
    if (node && node->type == VFS_TYPE_FILE) return node;
    return NULL;
}