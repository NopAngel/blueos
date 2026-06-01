#ifndef SLAB_H
#define SLAB_H

#include <stdint.h>
#include <stddef.h>

#define SLAB_NAME_MAX 32

typedef struct slab_s {
    void* s_mem;               /* Dirección de inicio de los objetos */
    struct slab_s* next;       /* Siguiente slab en el cache */
    uint32_t free_bitmap;      /* Bitmap simple para rastrear slots libres (máx 32 por slab) */
    uint32_t objects_free;     /* Contador de slots disponibles */
} slab_t;

typedef struct kmem_cache_s {
    char name[SLAB_NAME_MAX];
    size_t obj_size;           /* Tamaño de cada objeto */
    slab_t* slabs_full;        /* Lista de slabs sin espacio */
    slab_t* slabs_partial;     /* Lista de slabs con algunos huecos */
    slab_t* slabs_free;        /* Lista de slabs vacíos */
    
    struct kmem_cache_s* next; /* Lista global de caches */
} kmem_cache_t;

/* API Pública */
void slab_init();
kmem_cache_t* kmem_cache_create(const char* name, size_t size);
void* kmem_cache_alloc(kmem_cache_t* cache);
void kmem_cache_free(kmem_cache_t* cache, void* obj);

#endif