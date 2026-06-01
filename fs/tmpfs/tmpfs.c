#include <drivers/tmpfs.h>
#include <kernel/malloc.h>
#include <mm/memory.h>
#include <lib/string.h>
#include <kernel/printk.h>

static tmp_node_t *tmp_root = NULL;

void tmp_init() {
    tmp_root = NULL;
    boot_msg("TMPFS", "Volatile memory storage driver active.\n", 0);
}

int tmp_add_file(const char *name, const void *data, uint32_t size) {
    uint32_t dummy;
    if (tmp_get_file(name, &dummy) != NULL) {
        tmp_remove_file(name);
    }

    tmp_node_t *new_node = (tmp_node_t*)kmalloc(sizeof(tmp_node_t));
    if (!new_node) return -1;

    strncpy(new_node->name, name, 31);
    new_node->buffer = kmalloc(size);
    if (!new_node->buffer) {
        // kfree(new_node); 
        return -1;
    }

    memcpy(new_node->buffer, data, size);
    new_node->size = size;
    new_node->next = tmp_root;
    tmp_root = new_node;
    return 0;
}

void* tmp_get_file(const char *name, uint32_t *out_size) {
    tmp_node_t *curr = tmp_root;
    while (curr) {
        if (strcmp(curr->name, name) == 0) {
            if (out_size) *out_size = curr->size;
            return curr->buffer;
        }
        curr = curr->next;
    }
    return NULL;
}

int tmp_remove_file(const char *name) {
    tmp_node_t *curr = tmp_root;
    tmp_node_t *prev = NULL;

    while (curr) {
        if (strcmp(curr->name, name) == 0) {
            if (prev) prev->next = curr->next;
            else tmp_root = curr->next;

            // kfree(curr->buffer);
            // kfree(curr);
            return 0;
        }
        prev = curr;
        curr = curr->next;
    }
    return -1;
}

void tmp_list() {
    tmp_node_t *curr = tmp_root;
    printk("\nTMP Filesystem contents:\n");
    if (!curr) {
        printk("  (empty)\n");
        return;
    }
    while (curr) {
        printk("  %-20s %d bytes\n", curr->name, curr->size);
        curr = curr->next;
    }
    printk("\n");
}