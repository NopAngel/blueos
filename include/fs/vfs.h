#ifndef VFS_H
#define VFS_H

#include <stdint.h>
#include <stddef.h>

#define VFS_NAME_MAX 32
#define VFS_MAX_CONTENT 200


typedef enum {
    VFS_TYPE_FILE = 1,
    VFS_TYPE_DIR,
    VFS_TYPE_CHARDEV,
    VFS_TYPE_BLKDEV,
    VFS_TYPE_MOUNTPOINT
} vfs_type_t;

struct vfs_node;

typedef struct {
    int (*open)(struct vfs_node* node, uint32_t flags);
    int (*close)(struct vfs_node* node);
    int (*read)(struct vfs_node* node, void* buffer, uint32_t size, uint32_t offset);
    int (*write)(struct vfs_node* node, const void* buffer, uint32_t size, uint32_t offset);
    struct vfs_node* (*finddir)(struct vfs_node* node, const char* name);
    int (*mkdir)(struct vfs_node* node, const char* name, uint16_t mode);
    int (*readdir)(struct vfs_node* node, uint32_t index, struct vfs_dirent* dirent_out);
} vfs_ops_t;



// vnode/inode
typedef struct vfs_node {
    char name[VFS_NAME_MAX];
    uint32_t inode;
    uint32_t size;
    uint32_t flags;
    vfs_type_t type;
    vfs_ops_t* ops;
    void* private_data;
    struct vfs_node* ptr;
    struct vfs_node* parent;
} vfs_node_t;

struct vfs_dirent {
    char name[VFS_NAME_MAX];
    uint32_t inode;
    uint32_t type;
};

void vfs_init(void);
vfs_node_t* vfs_get_root(void);
int vfs_read(vfs_node_t* node, void* buffer, uint32_t size, uint32_t offset);
int vfs_write(vfs_node_t* node, const void* buffer, uint32_t size, uint32_t offset);
vfs_node_t* vfs_lookup(const char* path);

#endif
