/* fs/adfs/adfs_fs.c */

#include <stdint.h>
#include <fs/adfs/adfs.h>
#include <fs/vfs.h>
#include <kernel/printk.h>

int adfs_fill_super(const char *source, const char *target) {
    return 0;
}

char* adfs_driver_read(const char *path) {
    static char *msg = "ADFS: File content from Acorn disk (Stub)";
    return msg;
}

void adfs_init(void) {
    static struct vfs_driver adfs_driver = {
        .name = "adfs",
        .mount = adfs_fill_super
    };

}