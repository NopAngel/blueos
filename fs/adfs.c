/* fs/adfs/adfs_fs.c */

#include <stdint.h>
#include <fs/adfs/adfs.h>
#include <fs/vfs.h>
#include <blueos/printk.h>

int adfs_fill_super(const char *source, const char *target) {
    // Aquí va tu lógica de leer el sector 0 que hicimos antes
    return 0;
}

char* adfs_driver_read(const char *path) {
    // Aquí después pondremos la lógica de leer sectores del disco
    static char *msg = "ADFS: File content from Acorn disk (Stub)";
    return msg;
}

void adfs_init(void) {
    // Ahora sí, el compilador sabe que tiene .name y .mount
    static struct vfs_driver adfs_driver = {
        .name = "adfs",
        .mount = adfs_fill_super
    };

    vfs_register_driver(&adfs_driver);
}