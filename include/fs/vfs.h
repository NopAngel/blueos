#ifndef _BLUEOS_VFS_H_
#define _BLUEOS_VFS_H_

#include <stdint.h>
#include <stddef.h>

#define VFS_NAME_MAX 256

/* Forward declarations to prevent circular compilation reference issues */
struct vfs_node;

/* Operational function pointer table interface for filesystem drivers */
typedef struct vfs_ops {
    int (*open)(struct vfs_node *node, int flags);
    int (*close)(struct vfs_node *node);
    int (*read)(struct vfs_node *node, char *buffer, uint32_t size, uint32_t offset);
    int (*write)(struct vfs_node *node, const char *buffer, uint32_t size, uint32_t offset);
    int (*mkdir)(struct vfs_node *node, const char *name, uint16_t mode);
    int (*create)(struct vfs_node *node, const char *name, const char *flags);
    int (*unlink)(struct vfs_node *node, const char *name);
    int (*readdir)(struct vfs_node *node, uint32_t index, void *dirent_out);
    struct vfs_node* (*finddir)(struct vfs_node *parent, const char *name);

    int (*ioctl)(struct vfs_node *node, uint64_t request, void *arg);
	uintptr_t (*mmap)(struct vfs_node *node, uintptr_t addr, size_t length, int prot, int flags, size_t pgoffset);
} vfs_ops_t;

/* Core data structure representing a generalized node abstraction (Vnode) */
typedef struct vfs_node {
    char name[VFS_NAME_MAX];
    uint32_t type;
    uint32_t inode;
    uint32_t size;
    uint32_t start_block;
    uintptr_t ram_address;       /* Mapped address tracking for raw memory file loads */
    struct vfs_node *parent;
    vfs_ops_t *ops;              /* Refactored to reference the explicit type abstraction */
    void *ptr;                   /* Internal data block references or lists pointer */
    struct vfs_node *next;       /* RESTORED: Preserves backward compatibility for linked lists */
} vfs_node_t;

/* Generic Directory Entry packaging structure mapped into user space or shells */
struct vfs_dirent {
    char name[VFS_NAME_MAX];
    uint32_t type;
    uint32_t inode;
};

/* Explicit type flags classifications */
#define VFS_TYPE_FILE 1
#define VFS_TYPE_DIR  2

int vfs_mkdir(const char *path, uint16_t mode);
vfs_node_t *vfs_findfile(const char *path);
int vfs_touch(const char *path, const char *content);
vfs_node_t *vfs_lookup(const char *path);
int vfs_chdir(const char *path);
void vfs_init(void);
int vfs_rm(const char *path);
vfs_node_t *vfs_get_current(void);
int vfs_readdir(vfs_node_t *node, uint32_t index, struct vfs_dirent *dirent);
void vfs_get_cwd(char *buffer, uint32_t size);
int vfs_unlink(const char *path);
int vfs_read(vfs_node_t *node, void *buffer, uint32_t size, uint32_t offset);
vfs_node_t *vfs_finddir(vfs_node_t *node, const char *name);
vfs_node_t *vfs_get_root(void);
int vfs_create(const char *path, const char *content);
void vfs_register_fb0(void *arch_data);

#endif /* _BLUEOS_VFS_H_ */