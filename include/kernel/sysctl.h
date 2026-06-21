#ifndef SYSCTL_H
#define SYSCTL_H

#include <stdint.h>
#include <stdbool.h>

#define SYSCTL_TYPE_INT    0
#define SYSCTL_TYPE_STRING 1

typedef struct {
    const char *name;
    void *value;
    uint8_t type;
    uint8_t writable;
} sysctl_entry_t;

void sysctl_list(void);
int sysctl_set(const char *name, const char *new_value);
int sysctl_get(const char *name, void *out_buffer, int type_expected);

#endif