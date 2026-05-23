#ifndef PANIC_H
#define PANIC_H

void k_panic(const char *file, int line, const char *reason, ...);

#define PANIC(msg) k_panic(__FILE__, __LINE__, msg, ..)

#endif