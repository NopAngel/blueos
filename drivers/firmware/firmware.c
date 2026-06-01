#include <drivers/firmware.h>
#include <kernel/printk.h>
#include <lib/string.h>


int request_firmware(const struct firmware **fw, const char *name) {
    printk("[FW] Searching for firmware: %s...\n", name);

    static struct firmware dummy_fw;
    strncpy(dummy_fw.name, name, 32);
    dummy_fw.size = 4096;
    // dummy_fw.data = (uint8_t*)0x...;

    *fw = &dummy_fw;
    return 0;
}

int fw_load_to_device(uintptr_t dev_addr, const struct firmware *fw) {
    volatile uint32_t *dest = (uint32_t *)dev_addr;

    printk("[FW] Loading %d bytes into device at 0x%p\n", fw->size, dev_addr);

    for (size_t i = 0; i < fw->size / 4; i++) {
        dest[0] = ((uint32_t*)fw->data)[i];
    }

    printk("FW: Firmware %s loaded successfully!\n", fw->name);
    return 0;
}
