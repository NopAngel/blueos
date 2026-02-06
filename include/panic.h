// include/panic.h
#ifndef PANIC_H
#define PANIC_H


void k_panic(const char* msg, const char* file, int line);

#define k_panic(msg) k_panic(msg, __FILE__, __LINE__)

#endif