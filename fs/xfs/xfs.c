#include <fs/xfs.h>
#include <kernel/printk.h>
#include <fs/vfs.h>


static xfs_ag_t groups[AG_COUNT];
void xfs_init(void);
void xfs_create(const char* name);
void xfs_ls(void);

void xfs_init() {
    for(int i = 0; i < AG_COUNT; i++) {
        groups[i].ag_id = i;
        groups[i].free_blocks = 1024;
        groups[i].inode_count = 0;
    }
    printk(GREEN, "[  OK  ] BlueXFS: %d Allocation Groups initialized.\n", AG_COUNT);
}

int xfs_pick_ag() {
    int best_ag = 0;
    for(int i = 1; i < AG_COUNT; i++) {
        if(groups[i].free_blocks > groups[best_ag].free_blocks) {
            best_ag = i;
        }
    }
    return best_ag;
}

void xfs_create(const char* name) {
    int ag_id = xfs_pick_ag();
    xfs_ag_t *ag = &groups[ag_id];

    printk(CYAN, "[ XFS ] Creating '%s' in AG %d\n", name, ag_id);
    
    ag->inode_count++;
    ag->free_blocks--;
}

void xfs_ls(void) {
    printk(WHITE, "XFS Allocation Group Status:\n");
    for(int i = 0; i < AG_COUNT; i++) {
        printk(CYAN, " AG %d: %d inodes, %d free blocks\n", 
               groups[i].ag_id, groups[i].inode_count, groups[i].free_blocks);
    }
}

fs_ops_t xfs_ops = {
    .init  = xfs_init,
    .mkdir = xfs_create,
    .touch = xfs_create,
    .ls    = xfs_ls
};