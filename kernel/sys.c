#include <sys.h>

extern struct sysinit _sysinit_start[];
extern struct sysinit _sysinit_end[];

void sysinit_run(void) {
  for (struct sysinit *s = _sysinit_start; s < _sysinit_end; s++) {
    if (s->func != 0) {
      s->func();
    }
  }
}