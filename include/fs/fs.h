#ifndef FS_H
#define FS_H

void create_new_file(const char *filename, const char *content);
int touch(const char *filename, const char *content);
int mkdir(const char *dirname);
void list_items();
int cat(const char *filename);
void pwd();
int cd(const char *dirname);

#endif