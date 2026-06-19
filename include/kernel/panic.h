#ifndef PANIC_H
#define PANIC_H

void k_panic(int code, const char *reason);

#define PANIC(msg) k_panic(__FILE__, __LINE__, msg, ..)

#endif