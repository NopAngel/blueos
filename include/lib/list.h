#ifndef LIST_H
#define LIST_H

#include <stddef.h>
#include <stdint.h>

typedef struct node {
  struct node *next;
  struct node *prev;
  void *value;
} node_t;

typedef struct {
  node_t *head;
  node_t *tail;
  uint32_t length;
} list_t;

list_t *list_create();
void list_insert(list_t *list, void *value);
node_t *list_find(list_t *list, void *value);
void list_delete(list_t *list, node_t *node);

#define foreach(node, list)                                                    \
  for (node_t *node = (list)->head; node != NULL; node = node->next)

#endif