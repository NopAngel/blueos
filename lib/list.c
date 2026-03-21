#include <lib/list.h>
#include <kernel/mm/heap.h>

list_t *list_create() {
    list_t *out = malloc(sizeof(list_t));
    out->head = out->tail = NULL;
    out->length = 0;
    return out;
}

void list_insert(list_t *list, void *value) {
    node_t *node = malloc(sizeof(node_t));
    node->value = value;
    node->next = NULL;
    node->prev = list->tail;
    
    if (!list->head) list->head = node;
    if (list->tail) list->tail->next = node;
    list->tail = node;
    list->length++;
}

void list_delete(list_t *list, node_t *node) {
    if (node == list->head) list->head = node->next;
    if (node == list->tail) list->tail = node->prev;
    if (node->prev) node->prev->next = node->next;
    if (node->next) node->next->prev = node->prev;
    
    list->length--;
}