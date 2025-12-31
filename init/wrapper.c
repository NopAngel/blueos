// wrapper.c
#include <include/fs/vfs.h>

int mkdir(const char *dirname) {
    return vfs_mkdir(dirname);
}

int touch(const char *filename, const char *content) {
    return vfs_create(filename, content);
}

void list_items(void) {
    vfs_ls();
}

int write_file(const char *filename, const char *content) {
    return vfs_write(filename, content);
}

char* read_file(const char *filename) {
    return vfs_read(filename);
}

int remove_file(const char *filename) {
    return vfs_delete(filename);
}

int change_dir(const char *path) {
    return vfs_cd(path);
}

char* current_dir(void) {
    return vfs_pwd();
}
