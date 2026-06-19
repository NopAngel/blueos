#include <fs/xfs.h>
#include <kernel/errno.h>
#include <kernel/printk.h>

#define MODULE_NAME "XFS_ALLOC"

extern xfs_ag_t g_xfs_groups[XFS_AG_COUNT];

/**
 * xfs_pick_ag: Dynamically scans AG matrices to distribute writes across the
 * disk surface.
 */
int xfs_pick_ag(void) {
  int best_ag = 0;
  for (int i = 1; i < XFS_AG_COUNT; i++) {
    if (g_xfs_groups[i].free_blocks > g_xfs_groups[best_ag].free_blocks) {
      best_ag = i;
    }
  }
  return best_ag;
}

/**
 * xfs_alloc_block: Claims a free block from the target allocation group index.
 */
int32_t xfs_alloc_block(int ag_id) {
  xfs_ag_t *ag = &g_xfs_groups[ag_id];

  if (ag->free_blocks == 0)
    return -1;

  /* Simple linear search on the block bitmap allocator */
  for (uint32_t i = 0; i < 1024; i++) {
    uint32_t byte_idx = i / 8;
    uint32_t bit_idx = i % 8;

    if (!(ag->block_bitmap[byte_idx] & (1 << bit_idx))) {
      ag->block_bitmap[byte_idx] |=
          (1 << bit_idx); /* Mark block as allocated */
      ag->free_blocks--;
      return (int32_t)i;
    }
  }
  return -1;
}