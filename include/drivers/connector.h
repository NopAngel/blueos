#ifndef CONNECTOR_H
#define CONNECTOR_H

#include <stdint.h>

#define DEV_CONNECTOR_BUF_SIZE 1024

typedef struct {
    char buffer[DEV_CONNECTOR_BUF_SIZE];
    uint32_t head;
    uint32_t tail;
    int is_open;
} connector_dev_t;

static connector_dev_t blue_connector;


int connector_write(const char* data, uint32_t size);

int connector_read(char* out_buf, uint32_t size);

#endif