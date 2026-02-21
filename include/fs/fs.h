#ifndef FS_H
#define FS_H

#define MAX_DIRECTORIES 300
#define MAX_FILES 300
#define MAX_NAME_LENGTH 30
#define MAX_CONTENT_LENGTH 1024



typedef struct {
    char name[MAX_NAME_LENGTH];
    unsigned int parent_dir;
    unsigned int start_block;
    unsigned int size;
    int is_vfs;        /* 1 = System/Virtual, 0 = User */
} DirectoryEntry;

typedef struct {
    char name[MAX_NAME_LENGTH];
    unsigned int parent_dir;
    unsigned int size;
    
    char content[MAX_CONTENT_LENGTH];
    int is_vfs;        /* 1 = System/Virtual, 0 = User */
} FileEntry;


void create_new_file(const char *filename, const char *content);
int touch(const char *filename, const char *content);
int mkdir(const char *dirname);
void list_items();
void pwd();
int cd(const char *dirname);
void cat (const char *filename);
void fs_rm(const char *name);
void fs_rmdir(const char *name);
void fs_grep(const char *keyword, const char *filename);

#endif