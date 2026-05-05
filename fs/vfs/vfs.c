#include <fs/vfs.h>
#include <lib/string.h>
#include <kernel/malloc.h>
#include <kernel/printk.h>
#include <kernel/colors.h>

static vfs_node_t* vfs_root = NULL;
static vfs_node_t* vfs_current = NULL;

int vfs_read(vfs_node_t* node, void* buffer, uint32_t size, uint32_t offset) {
    if (node && node->ops && node->ops->read) {
        return node->ops->read(node, buffer, size, offset);
    }
    return -1;
}

vfs_node_t* vfs_get_current(void) {
    return vfs_current;
}

int root_readdir(struct vfs_node* node, uint32_t index, struct vfs_dirent* dirent_out) {
    return -1;
}
vfs_ops_t root_ops = {
    .readdir = root_readdir,
    .finddir = NULL,
    .read = NULL,
    .write = NULL
};


int vfs_mount(const char* dev, const char* target) {
    vfs_node_t* mount_point = vfs_lookup(target);
    if (!mount_point) return -1;

    printk(CYAN, "VFS: Mounting %s on %s\n", dev, target);

    return 0;
}

void vfs_init(void) {
    vfs_root = (vfs_node_t*)kmalloc(sizeof(vfs_node_t));
    memset(vfs_root, 0, sizeof(vfs_node_t));

    strcpy(vfs_root->name, "/");
    vfs_root->type = VFS_TYPE_DIR;
    vfs_root->ops = &root_ops;

    vfs_current = vfs_root;
}

void vfs_list_dir(const char* path) {
    vfs_ls(path);
}

void vfs_get_cwd(char* buffer, uint32_t size) {
    vfs_node_t* curr = vfs_get_current();
    if (curr) {
        strncpy(buffer, curr->name, size);
    } else {
        strncpy(buffer, "/", size);
    }
}

int vfs_chdir(const char* path) {
    vfs_node_t* node = vfs_lookup(path);
    if (node && node->type == VFS_TYPE_DIR) {
        vfs_current = node;
        return 0;
    }
    return -1;
}

int vfs_remove(const char* path) {
    vfs_node_t* node = vfs_lookup(path);
    if (!node) return -1;

    return -1;
}
