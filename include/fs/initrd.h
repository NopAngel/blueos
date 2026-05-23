#ifndef _INITRD_H
#define _INITRD_H

#include <stdint.h>

struct initrd_file {
    char name[64];
    uint32_t size;
    uint32_t offset;
} __attribute__((packed));

struct initrd_header {
    uint32_t nfiles;
    struct initrd_file files[32];
} __attribute__((packed));

void initrd_init(uint32_t location, uint32_t end);
void* initrd_read_file(const char* filename, uint32_t* size);

#endif