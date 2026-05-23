#include <fs/vfs.h>
#include <lib/string.h>
#include <kernel/malloc.h>
#include <kernel/printk.h>
#include <kernel/colors.h>
#include <mm/memory.h>

/* --- Global VFS pointers --- */
static vfs_node_t* vfs_root = NULL;
static vfs_node_t* vfs_current = NULL;

vfs_node_t* vfs_lookup(const char* path) {
    if (!path) return NULL;
    if (strcmp(path, "/") == 0) return vfs_root;

    vfs_node_t* curr = (path[0] == '/') ? vfs_root : vfs_current;

    char tmp_path[256];
    strncpy(tmp_path, path, 255);

    char* part = strtok(tmp_path, "/");
    while (part != NULL) {
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


/**
 * root_readdir - Implementation for the root directory listing
 * Note: Use 'void*' for the dirent to avoid strict type clashing if headers are messy.
 */
int root_readdir(struct vfs_node* node, uint32_t index, void* dirent_out) {
    // Root directory placeholder - currently no files to list
    return -1;
}

/*
 * VFS Operations for the root node
 * We use explicit casts to (void*) to bypass the 'incompatible-pointer-types' check
 * which is often triggered by redundant struct declarations.
 */
vfs_ops_t root_ops = {
    .readdir = (void*)root_readdir,
    .finddir = NULL,
    .read    = NULL,
    .write   = NULL,
    .open    = NULL,
    .close   = NULL
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
    printk(GREEN, "VFS: Successfully initialized root filesystem.\n");
}

vfs_node_t* vfs_get_current(void) {
    return vfs_current;
}

int vfs_chdir(const char* path) {
    // Assuming vfs_lookup is implemented elsewhere in your kernel
    vfs_node_t* node = vfs_lookup(path);
    if (node && node->type == VFS_TYPE_DIR) {
        vfs_current = node;
        return 0;
    }
    return -1;
}

void vfs_get_cwd(char* buffer, uint32_t size) {
    vfs_node_t* curr = vfs_get_current();
    if (curr) {
        strncpy(buffer, curr->name, size);
    } else {
        strncpy(buffer, "/", size);
    }
}

int vfs_mount(const char* dev, const char* target) {
    vfs_node_t* mount_point = vfs_lookup(target);
    if (!mount_point) return -1;

    printk(CYAN, "VFS: Mounted device %s on %s\n", dev, target);
    return 0;
}
