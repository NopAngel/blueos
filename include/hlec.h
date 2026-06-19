#ifndef EVENT_H
#define EVENT_H

#include <stdint.h>

typedef enum {
  EVENT_NONE = 0,
  EVENT_KEYBOARD,
  EVENT_MOUSE_CLICK,
  EVENT_MOUSE_MOVE,
  EVENT_SYSCALL,
  EVENT_TIMER
} event_type_t;

typedef struct {
  event_type_t type;
  uint32_t data1;
  uint32_t data2;
  uint32_t timestamp;
} event_t;

#define EVENT_QUEUE_SIZE 256

void hlec_init();
void hlec_push(event_t e);
event_t hlec_pop();

#endif