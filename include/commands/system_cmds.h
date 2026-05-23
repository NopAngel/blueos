#ifndef SYSTEM_CMDS_H
#define SYSTEM_CMDS_H

#include <stdint.h>

int cmd_uname(char* args);
int cmd_version(char* args);
int cmd_reboot(char* args);
int cmd_uptime(char* args);
int cmd_halt(char* args);
int cmd_dmesg(char* args);
int cmd_sysctl(char* args);
int cmd_vmstat(char* args);
int cmd_kldstat(char* args);
int cmd_kill(char* args);
int cmd_killall(char* args);
int cmd_time(char* args);
int cmd_lastlogin(char* args);
int cmd_jail(char* args);
int cmd_ls(char* args);
int cmd_cd(char* args);
int cmd_mkdir(char* args);
int cmd_rm(char* args);
int cmd_rmdir(char* args);
int cmd_touch(char* args);
int cmd_pwd(char* args);
int cmd_echo(char* args);
int cmd_sysctl(char* args);
int cmd_logout(char* args);
int cmd_halt(char* args);
int cmd_bluefetch(char* args);
int cmd_usr(char* args);
int cmd_chmod(char* args);
int cmd_whoami(char* args);
int cmd_env(char* args);
int cmd_set(char* args);
int cmd_battery(char* args);
int cmd_mdev(char* args);
int cmd_free(char* args);
int cmd_cat(char* args);
int cmd_mount(char* args);
int cmd_umount(char* args);
int cmd_fdisk(char* args);
int cmd_fsck(char* args);
int cmd_main(char* args);
int cmd_m4(char* args);
int cmd_stty(char* args);
int cmd_passwd(char* args);
int cmd_logname(char* args);
int cmd_dirname(char* path);
int cmd_basename(char* path);
int cmd_hostname(char* args);
int cmd_top(char* args);
int cmd_clear(char* args);
int cmd_alias(char* args);
int cmd_losetup(char* args);
int cmd_bc(char* args);
int cmd_cp(char* args);
int cmd_mv(char* args);
int cmd_i2c_scan(char* args);
int cmd_pci_scan(char* args);
int cmd_msg(char* args);
int cmd_sensors(char* args);
int cmd_help(char* args);

#endif /* SYSTEM_CMDS_H */
