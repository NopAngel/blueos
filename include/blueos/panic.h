#ifndef PANIC_H
#define PANIC_H

void k_panic(const char *reason, const char *file, int line);

#define PANIC(msg) k_panic(msg, __FILE__, __LINE__)

#endif