#include <kernel/malloc.h>
#include <kernel/printk.h>
#include <kernel/slab.h>
#include <lib/string.h>
#include <mm/memory.h>

extern void *pmm_alloc_frame();

static kmem_cache_t *cache_chain = NULL;

/**
 * Crea un nuevo cache para un tipo de objeto específico
 */
kmem_cache_t *kmem_cache_create(const char *name, size_t size) {
  kmem_cache_t *cache = (kmem_cache_t *)kmalloc(sizeof(kmem_cache_t));
  if (!cache)
    return NULL;

  strncpy(cache->name, name, SLAB_NAME_MAX);
  cache->obj_size = size;
  cache->slabs_full = NULL;
  cache->slabs_partial = NULL;
  cache->slabs_free = NULL;

  // Lo añadimos a la cadena global de caches
  cache->next = cache_chain;
  cache_chain = cache;

  printk("SLAB: Cache '%s' created (obj_size: %d bytes)\n", name, size);
  return cache;
}

/**
 * Crea un nuevo slab (página física) para un cache
 */
static slab_t *slab_create(kmem_cache_t *cache) {
  void *page = pmm_alloc_frame(); // Usando la función real del PMM
  if (!page)
    return NULL;

  slab_t *slab = (slab_t *)kmalloc(sizeof(slab_t));
  slab->s_mem = page;
  slab->next = NULL;
  slab->free_bitmap = 0;

  uint32_t max_objs = 4096 / cache->obj_size;
  if (max_objs > 32)
    max_objs = 32;

  slab->objects_free = max_objs;
  return slab;
}

/**
 * Reserva un objeto del cache
 */
void *kmem_cache_alloc(kmem_cache_t *cache) {
  slab_t *slab = cache->slabs_partial;

  if (!slab) {
    if (cache->slabs_free) {
      slab = cache->slabs_free;
      cache->slabs_free = slab->next;
    } else {
      slab = slab_create(cache);
    }
    slab->next = cache->slabs_partial;
    cache->slabs_partial = slab;
  }

  for (uint32_t i = 0; i < 32; i++) {
    if (!(slab->free_bitmap & (1 << i))) {
      slab->free_bitmap |= (1 << i);
      slab->objects_free--;

      void *obj = (void *)((uint32_t)slab->s_mem + (i * cache->obj_size));

      if (slab->objects_free == 0) {
        cache->slabs_partial = slab->next;
        slab->next = cache->slabs_full;
        cache->slabs_full = slab;
      }

      return obj;
    }
  }

  return NULL;
}

void kmem_cache_free(kmem_cache_t *cache, void *obj) {
  if (!cache || !obj)
    return;

  slab_t *slab = NULL;
  slab_t **prev_link = NULL;

  slab = cache->slabs_full;
  prev_link = &cache->slabs_full;

  while (slab) {
    uintptr_t addr = (uintptr_t)obj;
    uintptr_t start = (uintptr_t)slab->s_mem;
    uintptr_t end = start + 4096;

    if (addr >= start && addr < end) {
      uint32_t idx = (addr - start) / cache->obj_size;

      slab->free_bitmap &= ~(1 << idx);
      slab->objects_free++;

      if (slab->objects_free == 1) {
        *prev_link = slab->next;
        slab->next = cache->slabs_partial;
        cache->slabs_partial = slab;
      } else if (slab->objects_free == (4096 / cache->obj_size)) {

        *prev_link = slab->next;
        slab->next = cache->slabs_free;
        cache->slabs_free = slab;
      }

      return;
    }
    prev_link = &slab->next;
    slab = slab->next;
  }
}
