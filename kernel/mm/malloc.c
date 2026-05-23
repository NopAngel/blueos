#include <kernel/malloc.h>
#include <kernel/printk.h>

static header_t *heap_start = NULL;

void kmalloc_init(uintptr_t start, size_t size) {
    uintptr_t aligned_start = (start + 7) & ~7;
    size -= (aligned_start - start);

    heap_start = (header_t *)aligned_start;
    heap_start->size = size - sizeof(header_t);
    heap_start->next = NULL;
    heap_start->is_free = 1;

    printk(WHITE, "[MM] Heap initialized at %p (%d KB)\n", (void*)aligned_start, size / 1024);
}


void kfree(void* ptr) {
    if (!ptr) return;

    header_t *header = (header_t *)ptr - 1;
    header->is_free = 1;

    header_t *curr = heap_start;
    while (curr && curr->next) {
        if (curr->is_free && curr->next->is_free) {
            curr->size += sizeof(header_t) + curr->next->size;
            curr->next = curr->next->next;
            continue;
        }
        curr = curr->next;
    }
}
