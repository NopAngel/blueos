#include <stdint.h>
#include <mm/pmm.h>
#include <mm/memory.h>
#include <lib/string.h>
#include <stddef.h>

struct ashmem_area {
    char name[32];
    size_t size;
    void* phys_addr;
};

int ashmem_create_region(const char *name, size_t size) {
    struct ashmem_area *area = kmalloc(sizeof(struct ashmem_area));

    area->phys_addr = kalloc_page();
    area->size = size;
    strncpy(area->name, name, 32);

    return 0;
}
