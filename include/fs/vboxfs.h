#ifndef VBOXFS_H
#define VBOXFS_H

#include <stdint.h>

#define VBOX_VMMDEV_VERSION 0x00010001
#define VBOX_VMMDEV_PKT_HGCM_CALL 60

typedef struct {
    uint32_t size;
    uint32_t version;
    uint32_t type;
    int32_t  rc;
    uint32_t reserved1;
    uint32_t reserved2;
} vbox_header_t;

typedef struct {
    vbox_header_t header;
    uint32_t client_id;
    uint32_t function;
    uint32_t param_count;
} vbox_hgcm_call_t;

void vboxfs_init(uint32_t ioport);

#endif