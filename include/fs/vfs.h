#ifndef __VFS_H__
#define __VFS_H__

#include "../printk.h"
#include "../colors.h"
#include "../string/string.h"

#define VFS_MAX_PATH 256
#define VFS_MAX_NAME 30
#define VFS_MAX_CONTENT 1024
#define VFS_MAX_ENTRIES 500
#define VFS_MAX_OPEN_FILES 32


#ifndef NULL
#define NULL ((void*)0)
#endif

typedef enum {
    VFS_TYPE_FILE,
    VFS_TYPE_DIRECTORY
} vfs_entry_type;

typedef struct {
    char name[VFS_MAX_NAME];
    vfs_entry_type type;
    unsigned int parent;          
    unsigned int size;           
    unsigned int inode;           
    unsigned int data_block;     
    unsigned int created_time;   
    unsigned int modified_time;
} vfs_entry;

typedef struct {
    vfs_entry entries[VFS_MAX_ENTRIES];
    unsigned int entry_count;
    unsigned int current_directory; 
    unsigned int root_directory;    
} vfs_system;


typedef struct {
    unsigned int inode;
    unsigned int position;  
    unsigned int mode;      
    unsigned int ref_count;
} vfs_file_handle;


void vfs_init(void);
int vfs_mkdir(const char *name);
int vfs_create(const char *name, const char *content);
int vfs_write(const char *name, const char *content);
char* vfs_read(const char *name);
int vfs_delete(const char *name);
int vfs_cd(const char *path);
void vfs_ls(void);
char* vfs_pwd(void);
int vfs_exists(const char *path);
int vfs_open(const char *name, unsigned int mode);
int vfs_close(int fd);
int vfs_read_fd(int fd, char *buffer, unsigned int size);
int vfs_write_fd(int fd, const char *buffer, unsigned int size);
vfs_entry* vfs_find_entry(const char *name, vfs_entry_type type);
vfs_entry* vfs_get_entry_by_inode(unsigned int inode);


static void vfs_memset(void *ptr, char value, unsigned int size);
static void vfs_memcpy(void *dest, const void *src, unsigned int size);
static void vfs_strcat(char *dest, const char *src);

#endif // __VFS_H__