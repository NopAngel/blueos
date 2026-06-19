#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
  uint8_t *buffer;
  size_t size;
  size_t read_ptr;
  size_t write_ptr;
} ring_buffer_t;

ring_buffer_t *ring_buffer_create(size_t size);
size_t ring_buffer_write(ring_buffer_t *rb, size_t size, uint8_t *data);
size_t ring_buffer_read(ring_buffer_t *rb, size_t size, uint8_t *data);
size_t ring_buffer_unread(ring_buffer_t *rb);
void ring_buffer_destroy(ring_buffer_t *rb);

#endif