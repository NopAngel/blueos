#ifndef _LIBUSER_H
#define _LIBUSER_H

// Solo el prototipo
void user_print(const char *str);
void user_exit();
int syscall(int number, int arg1);
int vfs_chdir(const char *path);

#endif