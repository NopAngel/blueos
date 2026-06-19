#ifndef _BLUEOS_JFS_H
#define _BLUEOS_JFS_H

#include <stddef.h>

#define JFS_MAGIC 0x424C5545
#define BLOCK_SIZE 4096
#define JOURNAL_SIZE 128
#define MAX_FILES 64
#define TYPE_FILE 1
#define TYPE_DIR 2

typedef struct {
  unsigned int magic;
  unsigned int total_blocks;
  unsigned int free_blocks;
  unsigned int root_inode;
} jfs_superblock_t;

typedef struct {
  char name[32];
  unsigned int size;
  unsigned int type;
  unsigned int start_block;
  int used;
} jfs_inode_t;

typedef enum {
  JFS_TRANS_START,
  JFS_TRANS_COMMIT,
  JFS_TRANS_REVOKE
} jfs_status_t;

typedef struct {
  unsigned int transaction_id;
  jfs_status_t status;
  unsigned int target_block;
  char data[BLOCK_SIZE];
} jfs_journal_entry_t;

extern jfs_inode_t inode_table[MAX_FILES];

#endif