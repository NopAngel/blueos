#include <hlec.h>

static event_t event_queue[EVENT_QUEUE_SIZE];
uint32_t events = 0;
static uint32_t head = 0;
static uint32_t tail = 0;

void hlec_init() {
  head = 0;
  tail = 0;
  for (int i = 0; i < EVENT_QUEUE_SIZE; i++) {
    event_queue[i].type = EVENT_NONE;
  }
}

void hlec_push(event_t e) {
  uint32_t next = (head + 1) % EVENT_QUEUE_SIZE;
  events++;
  if (next != tail) {
    event_queue[head] = e;
    head = next;
  }
}

event_t hlec_pop() {
  event_t e = {.type = EVENT_NONE};
  if (head != tail) {
    e = event_queue[tail];
    tail = (tail + 1) % EVENT_QUEUE_SIZE;
  }
  return e;
}