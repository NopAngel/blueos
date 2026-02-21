#ifndef SYSFS_H
#define SYSFS_H

int sysfs_read_version(char *buffer);
int sysfs_read_uptime(char *buffer);
void sysfs_init();

#endif