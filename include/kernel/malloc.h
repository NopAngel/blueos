#ifndef MALLOC_H
#define MALLOC_H

#include <stdint.h>
#include <stddef.h>

typedef struct header {
    size_t size;
    struct header *next;
    int is_free;
} header_t;

void kmalloc_init(uintptr_t start, size_t size);
void kfree(void *ptr);


#endif
