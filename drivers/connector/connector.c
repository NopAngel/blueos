#include <drivers/connector.h>
#include <stdint.h>
#include <kernel/printk.h>
#include <kernel/colors.h>
#include <lib/string.h>

int connector_write(const char* data, uint32_t size) {
    if (size > DEV_CONNECTOR_BUF_SIZE) size = DEV_CONNECTOR_BUF_SIZE;

    memcpy(blue_connector.buffer, data, size);
    blue_connector.head = size;

    printk(YELLOW, "[CONNECTOR] The user says: %s\n", blue_connector.buffer);
    return size;
}

int connector_read(char* out_buf, uint32_t size) {
    uint32_t to_read = (size < blue_connector.head) ? size : blue_connector.head;

    memcpy(out_buf, blue_connector.buffer, to_read);
    return to_read;
}
