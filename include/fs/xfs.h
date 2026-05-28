#ifndef _BLUEOS_XFS_H
#define _BLUEOS_XFS_H

#define XFS_MAGIC 0x58465342 // "XFSB"
#define AG_COUNT 4           

typedef struct {
    unsigned int ag_id;
    unsigned int free_blocks;
    unsigned int inode_count;
    unsigned int btree_root;
} xfs_ag_t;

typedef struct {
    unsigned int inode_num;
    unsigned int mode;   
    unsigned int size;
    unsigned int nblocks;
    unsigned int extents[12];
} xfs_inode_t;

#endif