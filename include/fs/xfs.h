#ifndef BLUEOS_XFS_TYPES_H_
#define BLUEOS_XFS_TYPES_H_

#include <kernel/types.h>
#include <stdint.h>

#define XFS_AG_COUNT 4
#define XFS_BLOCK_SIZE 4096
#define XFS_MAX_FILES_AG 32
#define XFS_MAX_NAME 64

/* XFS Inode structure tracking attributes and data map bounds */
typedef struct {
    uint32_t inode_num;
    char name[XFS_MAX_NAME];
    uint32_t size;
    uint32_t type;
    uint32_t start_block;
    uint8_t is_active;
    uint32_t parent_inode;
    uintptr_t ram_address;
} xfs_inode_t;
/* Allocation Group definition acting as an independent sub-volume locking zone
 */
typedef struct {
  uint32_t ag_id;
  uint32_t free_blocks;
  uint32_t inode_count;
  xfs_inode_t inodes[XFS_MAX_FILES_AG];
  uint8_t block_bitmap[128]; /* Simplistic tracking layout for 1024 allocation
                                blocks */
} xfs_ag_t;

int xfs_vfs_create(struct vfs_node *node, const char *name, const char *flags);
void xfs_init(void);

#endif