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


struct vfs_dirent;

typedef struct {
    int (*open)(struct vfs_node* node, uint32_t flags);
    int (*close)(struct vfs_node* node);
    int (*read)(struct vfs_node* node, void* buffer, uint32_t size, uint32_t offset);
    int (*write)(struct vfs_node* node, const void* buffer, uint32_t size, uint32_t offset);
    struct vfs_node* (*finddir)(struct vfs_node* node, const char* name);
    int (*mkdir)(struct vfs_node* node, const char* name, uint16_t mode);
    int (*readdir)(struct vfs_node* node, uint32_t index, struct vfs_dirent* dirent_out);
    int (*create)(struct vfs_node* node, const char* name, const char* content);
    int (*unlink)(struct vfs_node* node, const char* name);
} vfs_ops_t;


typedef struct vfs_node {
    char name[VFS_NAME_MAX];
    uint32_t inode;
    uint32_t size;
    uint32_t type;
    uint32_t flags;
    struct vfs_node* parent;
    struct vfs_node* next;
    vfs_ops_t* ops;
    void* ptr;
} vfs_node_t;
struct vfs_dirent {
    char name[VFS_NAME_MAX];
    uint32_t inode;
    uint32_t type;
};

typedef struct vfs_mount {
    char mountpoint[16];  
    vfs_ops_t* ops;        
    struct vfs_mount* next;
} vfs_mount_t;

void vfs_init(void);
vfs_node_t* vfs_get_root(void);
int vfs_read(vfs_node_t* node, void* buffer, uint32_t size, uint32_t offset);
vfs_node_t* vfs_finddir(vfs_node_t* node, const char* name);
int vfs_write(vfs_node_t* node, const void* buffer, uint32_t size, uint32_t offset);
vfs_node_t* vfs_lookup(const char* path);
int vfs_mkdir(const char* path);
int vfs_create(const char* path, const char* content);
int vfs_touch(const char* path, const char* content);
int vfs_unlink(const char* path);
void vfs_get_cwd(char* buffer, uint32_t size);
vfs_node_t* vfs_get_current(void);
int vfs_chdir(const char* path);
int vfs_readdir(vfs_node_t* node, uint32_t index, struct vfs_dirent* dirent);
int vfs_rm(const char* path);
vfs_node_t* vfs_findfile(const char* path);

#endif
