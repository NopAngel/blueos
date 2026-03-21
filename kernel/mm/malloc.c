#include <kernel/malloc.h>
#include <kernel/printk.h>

static header_t *heap_start = NULL;

void kmalloc_init(uintptr_t start, size_t size) {
    heap_start = (header_t *)start;
    heap_start->size = size;
    heap_start->next = NULL;
    heap_start->is_free = 1;
    
    printk(WHITE, "[MM] Heap initialized at %p with size %d KB\n", start, size / 1024);
}

void *kmalloc(size_t size) {
    header_t *curr = heap_start;
    size_t total_size = size + sizeof(header_t);

    while (curr) {
        if (curr->is_free && curr->size >= total_size) {
            if (curr->size > total_size + sizeof(header_t) + 16) {
                header_t *new_block = (header_t *)((uintptr_t)curr + total_size);
                new_block->size = curr->size - total_size;
                new_block->next = curr->next;
                new_block->is_free = 1;

                curr->size = total_size;
                curr->next = new_block;
            }
            
            curr->is_free = 0;
            return (void *)(curr + 1);
        }
        curr = curr->next;
    }

    printk(RED, "KMALLOC: Out of memory!\n");
    return NULL;
}