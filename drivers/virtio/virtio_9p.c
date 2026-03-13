#include <stdbool.h>
#include <blueos/printk.h>

bool virtio_9p_present(void) {
    // return pci_find_device(0x1AF4, 0x1009); 
    return true; 
}

void v9p_init(void) {
    pr_info("9P: Initializing Virtio-9P transport layer...\n");

}