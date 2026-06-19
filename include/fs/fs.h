#ifndef FS_H
#define FS_H

#include <stdint.h>

#define LOOP_DEVICE_START 100
#define MAX_DIRECTORIES 300
#define MAX_FILES 300
#define MAX_NAME_LENGTH 30
#define MAX_CONTENT_LENGTH 1024

typedef struct {
  char name[MAX_NAME_LENGTH];
  unsigned int parent_dir;
  unsigned int start_block;
  unsigned int size;
  int is_vfs; /* 1 = System/Virtual, 0 = User */
} DirectoryEntry;

typedef struct {
  char name[MAX_NAME_LENGTH];
  unsigned int parent_dir;
  unsigned int size;
  uint32_t permissions;
  uint32_t owner_id;
  char content[MAX_CONTENT_LENGTH];
  int is_vfs; /* 1 = System/Virtual, 0 = User */
} FileEntry;

void ramfsinit_clean();
void ramfsinit();
int ramfs_mkdir(const char *dirname);
int ramfs_touch(const char *filename, const char *content);
void ramfslist_items();
int ramfs_cd(const char *dirname);
void ramfs_pwd();
void ramfs_cat(const char *filename);
void ramfs_rm(const char *name);
void ramfsrmdir(const char *name);
int ramfs_find_file(const char *name);
void ramfsread_at(const char *filename, uint32_t offset, uint32_t size,
                  char *buffer);

#endif
