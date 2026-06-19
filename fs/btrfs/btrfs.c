#include <drivers/ata.h>
#include <fs/btrfs.h>
#include <fs/vfs.h>
#include <kernel/malloc.h>
#include <kernel/printk.h>
#include <lib/string.h>
#include <stdbool.h>
#include <stdint.h>

extern void *kmalloc(uint32_t size);
extern void *mm_memset(void *s, int c, size_t n);

#define BLOCK_SIZE 512
#define BITMAP_BLOCK 1
#define INODE_BLOCK 0
#define MAX_BLOCKS 1024
#define CACHE_SLOTS 32

int btrfs_create_file(struct vfs_node *parent, const char *name,
                      const char *content);
int btrfs_read_file(struct vfs_node *node, char *buffer, uint32_t size,
                    uint32_t offset); /* Changed void* to char* */
int btrfs_write_file_vfs(struct vfs_node *node, const char *buffer,
                         uint32_t size, uint32_t offset); /* Changed const void* to const char* */
int btrfs_unlink(struct vfs_node *node, const char *name);
int btrfs_mkdir(struct vfs_node *parent, const char *name, uint16_t mode);
vfs_ops_t bluefs_ops = {.create = btrfs_create_file,
                        .read = btrfs_read_file,
                        .write = btrfs_write_file_vfs,
                        .unlink = btrfs_unlink,
                        .mkdir = btrfs_mkdir,
                        .readdir = NULL,
                        .finddir = NULL};

static buffer_slot_t cache[CACHE_SLOTS];

void *block_get(uint32_t block_id) {
  for (int i = 0; i < CACHE_SLOTS; i++) {
    if (cache[i].valid && cache[i].block_id == block_id) {
      return cache[i].data;
    }
  }

  for (int i = 0; i < CACHE_SLOTS; i++) {
    if (!cache[i].valid || !cache[i].dirty) {
      if (cache[i].dirty) {
        ata_write_sector(cache[i].block_id, cache[i].data);
      }

      ata_read_sector(block_id, cache[i].data);

      cache[i].block_id = block_id;
      cache[i].valid = true;
      cache[i].dirty = false;
      return cache[i].data;
    }
  }
  return NULL;
}

void block_mark_dirty(uint32_t block_id) {
  for (int i = 0; i < CACHE_SLOTS; i++) {
    if (cache[i].valid && cache[i].block_id == block_id) {
      cache[i].dirty = true;
      return;
    }
  }
}

struct btrfs_disk_inode *get_inode(uint32_t inode_id) {
  uint8_t *block_buffer = (uint8_t *)block_get(0);

  return (struct btrfs_disk_inode *)(block_buffer + (inode_id * 64));
}
struct journal_entry {
  uint32_t block_id;
  uint8_t data[BLOCK_SIZE];
} log_entry;

void journal_log(uint32_t block_id, void *data) {
  log_entry.block_id = block_id;
  memcpy(log_entry.data, data, BLOCK_SIZE);
}

int btrfs_write_file(uint32_t inode_id, const char *content, uint32_t len) {
  struct btrfs_disk_inode *inode = get_inode(inode_id);
  uint32_t block_id = inode->block_ptrs[0];

  journal_log(block_id, block_get(block_id));

  void *block = block_get(block_id);
  memcpy(block, content, len);
  block_mark_dirty(block_id);

  inode->size = len;
  block_mark_dirty(0);

  return 0;
}

void btrfs_flush_cache() {
  int writes = 0;
  for (int i = 0; i < CACHE_SLOTS; i++) {
    if (cache[i].valid && cache[i].dirty) {
      ata_write_sector(cache[i].block_id, cache[i].data);
      cache[i].dirty = false;
      writes++;
    }
  }
}

int btrfs_allocate_block() {
  uint8_t *bitmap = (uint8_t *)block_get(BITMAP_BLOCK);

  for (int i = 0; i < BLOCK_SIZE; i++) {
    for (int bit = 0; bit < 8; bit++) {
      if (!(bitmap[i] & (1 << bit))) {
        bitmap[i] |= (1 << bit);
        block_mark_dirty(BITMAP_BLOCK);
        return (i * 8) + bit;
      }
    }
  }
  return -1;
}

void btrfs_mount_disk() {
  uint8_t *block = (uint8_t *)block_get(INODE_BLOCK);

  struct btrfs_disk_inode *inode_table = get_inode(0);
  vfs_node_t *root = vfs_get_root();

  for (int i = 1; i < 8; i++) {
    if (inode_table[i].block_ptrs[0] != 0 && inode_table[i].name[0] != '\0') {
      vfs_node_t *node = (vfs_node_t *)kmalloc(sizeof(vfs_node_t));
      mm_memset(node, 0, sizeof(vfs_node_t));

      strncpy(node->name, inode_table[i].name, VFS_NAME_MAX);
      node->type = VFS_TYPE_FILE;
      node->inode = i;
      node->ops = &bluefs_ops;
      node->parent = root;
      node->next = root->ptr;
      root->ptr = node;
    }
  }
}

void btrfs_free_block(int block_id) {
  uint8_t *bitmap = (uint8_t *)block_get(BITMAP_BLOCK);
  int byte = block_id / 8;
  int bit = block_id % 8;

  bitmap[byte] &= ~(1 << bit);
  block_mark_dirty(BITMAP_BLOCK);
}

int btrfs_create_file(struct vfs_node *parent, const char *name,
                      const char *content) {
  struct btrfs_disk_inode *inode_table = get_inode(0);
  int free_inode = -1;

  for (int i = 1; i < 8; i++) {
    if (inode_table[i].block_ptrs[0] == 0) {
      free_inode = i;
      break;
    }
  }

  if (free_inode == -1) {
    boot_msg("BTRFS", "There are no free inodes in Block 0.", 2);
    return -1;
  }

  int block_id = btrfs_allocate_block();
  if (block_id == -1)
    return -1;

  struct btrfs_disk_inode *inode = &inode_table[free_inode];
  mm_memset(inode, 0, sizeof(struct btrfs_disk_inode));

  inode->block_ptrs[0] = block_id;
  inode->size = (content) ? strlen(content) : 0;

  strncpy(inode->name, name, 31);
  inode->name[31] = '\0';

  if (content) {
    btrfs_write_file(free_inode, content, strlen(content));
  } else {
    block_mark_dirty(0);
  }

  block_mark_dirty(BITMAP_BLOCK);
  btrfs_flush_cache();

  vfs_node_t *child = (vfs_node_t *)kmalloc(sizeof(vfs_node_t));
  mm_memset(child, 0, sizeof(vfs_node_t));
  strncpy(child->name, name, VFS_NAME_MAX);
  child->type = VFS_TYPE_FILE;
  child->inode = free_inode;
  child->ops = &bluefs_ops;
  child->parent = parent;
  child->next = parent->ptr;
  parent->ptr = child;

  return 0;
}

int btrfs_read_file(struct vfs_node *node, char *buffer, uint32_t size,
                    uint32_t offset) {
    struct btrfs_disk_inode *inode = get_inode(node->inode);
    void *block_data = block_get(inode->block_ptrs[0]);
    
    /* Direct bytes transference into character storage array mapping */
    memcpy(buffer, (char *)block_data + offset, size);
    return size;
}
int btrfs_write_file_vfs(struct vfs_node *node, const char *buffer,
                         uint32_t size, uint32_t offset) {
    struct btrfs_disk_inode *inode = get_inode(node->inode);
    void *block_data = block_get(inode->block_ptrs[0]);
    
    /* Commit payload from characters array into cached physical storage memory */
    memcpy((char *)block_data + offset, buffer, size);
    block_mark_dirty(inode->block_ptrs[0]);
    
    inode->size = size + offset;
    block_mark_dirty(0);
    return size;
}
int btrfs_unlink(struct vfs_node *node, const char *name) {
  struct btrfs_disk_inode *inode = get_inode(node->inode);
  btrfs_free_block(inode->block_ptrs[0]);
  mm_memset(inode, 0, sizeof(struct btrfs_disk_inode));
  block_mark_dirty(0);
  return 0;
}

int btrfs_mkdir(struct vfs_node *parent, const char *name, uint16_t mode) {
  vfs_node_t *child = (vfs_node_t *)kmalloc(sizeof(vfs_node_t));
  if (!child)
    return -1;
  mm_memset(child, 0, sizeof(vfs_node_t));

  strncpy(child->name, name, VFS_NAME_MAX);
  child->type = VFS_TYPE_DIR;
  child->parent = parent;
  child->ops = &bluefs_ops;

  child->next = parent->ptr;
  parent->ptr = child;

  return 0;
}