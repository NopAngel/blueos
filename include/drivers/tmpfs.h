#ifndef TMPFS_H
#define TMPFS_H

#include <stddef.h>
#include <stdint.h>

typedef struct tmp_node {
  char name[32];
  void *buffer;
  uint32_t size;
  struct tmp_node *next;
} tmp_node_t;

void tmp_init();
int tmp_add_file(const char *name, const void *data, uint32_t size);
void *tmp_get_file(const char *name, uint32_t *out_size);
void tmp_list();
int tmp_remove_file(const char *name);

#endif