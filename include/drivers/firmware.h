#ifndef FIRMWARE_H
#define FIRMWARE_H

#include <stdint.h>
#include <stddef.h>

struct firmware {
    size_t size;           
    const uint8_t *data;   
    char name[32];         
};

int request_firmware(const struct firmware **fw, const char *name);
void release_firmware(const struct firmware *fw);
int fw_load_to_device(uintptr_t dev_addr, const struct firmware *fw);

#endif