#include <kernel/colors.h>

#include <fs/vboxfs.h>
#include <kernel/ports.h>
#include <kernel/printk.h>
uint32_t vbox_ioport = 0;

void vbox_send_request(void* request) {
    outl(vbox_ioport, (uint32_t)request);
}
int vboxfs_mount(const char* folder_name) {
    printk(YELLOW, "VBoxFS: Attempting to mount folder: %s...\n", folder_name);

    return 0; 
}

void vboxfs_init(uint32_t ioport) {
    vbox_ioport = ioport;

    static vbox_header_t req; 
    req.size = sizeof(vbox_header_t);
    req.version = VBOX_VMMDEV_VERSION;
    req.type = 30; 
    
    outl(vbox_ioport, (uint32_t)&req);
    
    if (req.rc >= 0) {
        printk(GREEN, "VBoxFS: Communication established.\n");
    }
}