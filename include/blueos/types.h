#ifndef _BLUEOS_TYPES_H
#define _BLUEOS_TYPES_H

#define NULL 0

/* 1. TIPOS BASE (Usa los nombres internos de GCC para evitar conflictos) */
typedef __UINT8_TYPE__   uint8_t;
typedef __UINT16_TYPE__  uint16_t;
typedef __UINT32_TYPE__  uint32_t;
typedef __UINT64_TYPE__  uint64_t;

typedef __INT8_TYPE__    int8_t;
typedef __INT16_TYPE__   int16_t;
typedef __INT32_TYPE__   int32_t;
typedef __INT64_TYPE__   int64_t;

/* 2. TIPOS DE PUNTERO Y TAMAÑO */
typedef __INTPTR_TYPE__  intptr_t;
typedef __UINTPTR_TYPE__ uintptr_t;
typedef __SIZE_TYPE__    size_t;
typedef __PTRDIFF_TYPE__ ptrdiff_t;

/* 3. TIPOS LÓGICOS */
typedef int64_t          ssize_t;
typedef uint32_t         pid_t;

#endif