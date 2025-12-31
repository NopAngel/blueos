#define MEMORY_C
#include <include/mm/memory.h>
#include <include/printk.h>
#include <include/colors.h>

extern int cursor_y;

/* Variables globales */
static struct memory_manager mm = {0};
static uint8_t bitmap_storage[BITMAP_SIZE];

/*----------------------------------------------------------------
 * Funciones de utilidad sin librerías
 *----------------------------------------------------------------*/

/* mm_memset - Versión propia de memset */
void mm_memset(void* ptr, uint8_t value, uint32_t size) {
    uint8_t* p = (uint8_t*)ptr;
    for (uint32_t i = 0; i < size; i++) {
        p[i] = value;
    }
}

/* mm_memcpy - Versión propia de memcpy */
void mm_memcpy(void* dest, const void* src, uint32_t size) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;

    for (uint32_t i = 0; i < size; i++) {
        d[i] = s[i];
    }
}

/* mm_memmove - Versión propia de memmove */
void mm_memmove(void* dest, const void* src, uint32_t size) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;

    if (d < s) {
        for (uint32_t i = 0; i < size; i++) {
            d[i] = s[i];
        }
    } else {
        for (uint32_t i = size; i > 0; i--) {
            d[i-1] = s[i-1];
        }
    }
}

/*----------------------------------------------------------------
 * Funciones internas del gestor de memoria
 *----------------------------------------------------------------*/

/* set_bit - Establece un bit en el bitmap */
static void set_bit(uint32_t bit, int value) {
    uint32_t byte = bit / 8;
    uint32_t bit_in_byte = bit % 8;

    if (value) {
        mm.bitmap[byte] |= (1 << bit_in_byte);
    } else {
        mm.bitmap[byte] &= ~(1 << bit_in_byte);
    }
}

/* get_bit - Obtiene el valor de un bit */
static int get_bit(uint32_t bit) {
    uint32_t byte = bit / 8;
    uint32_t bit_in_byte = bit % 8;

    return (mm.bitmap[byte] >> bit_in_byte) & 1;
}

/* mark_pages - Marca un rango de páginas */
static void mark_pages(uint32_t start_page, uint32_t count, int used) {
    for (uint32_t i = 0; i < count; i++) {
        set_bit(start_page + i, used);
    }
}

/* find_free_pages - Encuentra páginas consecutivas libres */
static int find_free_pages(uint32_t count, uint32_t* result) {
    uint32_t consecutive = 0;
    uint32_t start = mm.next_free;

    for (uint32_t i = start; i < mm.total_pages; i++) {
        if (!get_bit(i)) {
            consecutive++;
            if (consecutive == count) {
                *result = i - count + 1;
                return 1; /* Encontrado */
            }
        } else {
            consecutive = 0;
        }
    }

    /* Buscar desde el inicio si no encontró */
    consecutive = 0;
    for (uint32_t i = 0; i < start; i++) {
        if (!get_bit(i)) {
            consecutive++;
            if (consecutive == count) {
                *result = i - count + 1;
                return 1; /* Encontrado */
            }
        } else {
            consecutive = 0;
        }
    }

    return 0; /* No encontrado */
}

/* calculate_memory - Calcula memoria total del sistema */
static uint32_t calculate_memory(struct multiboot_info* mbi) {
    if (!mbi) {
        /* 16MB por defecto si no hay info */
        return 16 * 1024 * 1024;
    }

    /* Verificar flag de memoria */
    if (!(mbi->flags & 1)) {
        return 16 * 1024 * 1024;
    }

    /* Memoria en KB a bytes: (mem_upper + 1024) * 1024 */
    uint32_t mem_kb = mbi->mem_upper;
    return (mem_kb + 1024) * 1024;
}

/*----------------------------------------------------------------
 * Funciones públicas del gestor
 *----------------------------------------------------------------*/

/* mm_init - Inicializa el gestor de memoria */
void mm_init(struct multiboot_info* mbi) {
    if (mm.initialized) {
        return;
    }

    /* Calcular memoria total */
    mm.total_memory = calculate_memory(mbi);

    /* Calcular páginas totales */
    mm.total_pages = mm.total_memory / PAGE_SIZE;
    if (mm.total_memory % PAGE_SIZE != 0) {
        mm.total_pages++;
    }

    /* Inicializar bitmap */
    mm.bitmap = bitmap_storage;
    mm.bitmap_size = (mm.total_pages + 7) / 8;
    if (mm.bitmap_size > BITMAP_SIZE) {
        mm.bitmap_size = BITMAP_SIZE;
        mm.total_pages = BITMAP_SIZE * 8;
        mm.total_memory = mm.total_pages * PAGE_SIZE;
    }

    /* Limpiar bitmap */
    mm_memset(mm.bitmap, 0, mm.bitmap_size);

    /* Marcar páginas inexistentes como ocupadas */
    uint32_t real_pages = mm.total_memory / PAGE_SIZE;
    for (uint32_t i = real_pages; i < mm.total_pages; i++) {
        set_bit(i, 1);
    }

    /* Marcar páginas del kernel como ocupadas */
    uint32_t kernel_start_page = KERNEL_START / PAGE_SIZE;
    uint32_t kernel_pages = (KERNEL_SIZE + PAGE_SIZE - 1) / PAGE_SIZE;

    mark_pages(kernel_start_page, kernel_pages, 1);

    /* Inicializar contadores */
    mm.free_pages = mm.total_pages - kernel_pages;
    mm.free_memory = mm.free_pages * PAGE_SIZE;
    mm.used_memory = mm.total_memory - mm.free_memory;
    mm.next_free = kernel_start_page + kernel_pages;

    mm.initialized = 1;
}

/* kmalloc - Asigna memoria */
void* kmalloc(uint32_t size) {
    if (!mm.initialized || size == 0) {
        return 0;
    }

    /* Calcular páginas necesarias */
    uint32_t pages_needed = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    uint32_t start_page;

    /* Buscar páginas libres */
    if (!find_free_pages(pages_needed, &start_page)) {
        return 0; /* Sin memoria */
    }

    /* Marcar páginas como ocupadas */
    mark_pages(start_page, pages_needed, 1);

    /* Actualizar contadores */
    mm.free_pages -= pages_needed;
    mm.free_memory = mm.free_pages * PAGE_SIZE;
    mm.used_memory = mm.total_memory - mm.free_memory;

    /* Actualizar next_free si es necesario */
    if (start_page == mm.next_free) {
        mm.next_free = start_page + pages_needed;
    }

    /* Devolver dirección física */
    return (void*)(start_page * PAGE_SIZE);
}

/* kfree - Libera memoria */
void kfree(void* ptr, uint32_t size) {
    if (!mm.initialized || !ptr || size == 0) {
        return;
    }

    uint32_t addr = (uint32_t)ptr;
    uint32_t start_page = addr / PAGE_SIZE;
    uint32_t pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;

    /* Verificar límites */
    if (start_page >= mm.total_pages) {
        return;
    }

    /* Marcar páginas como libres */
    mark_pages(start_page, pages, 0);

    /* Actualizar contadores */
    mm.free_pages += pages;
    mm.free_memory = mm.free_pages * PAGE_SIZE;
    mm.used_memory = mm.total_memory - mm.free_memory;

    /* Actualizar next_free si es necesario */
    if (start_page < mm.next_free) {
        mm.next_free = start_page;
    }
}

/* kcalloc - Asigna y limpia memoria */
void* kcalloc(uint32_t num, uint32_t size) {
    uint32_t total = num * size;
    void* ptr = kmalloc(total);

    if (ptr) {
        mm_memset(ptr, 0, total);
    }

    return ptr;
}

/* Funciones de consulta */
uint32_t mm_get_total(void) {
    return mm.total_memory;
}

uint32_t mm_get_free(void) {
    return mm.free_memory;
}

uint32_t mm_get_used(void) {
    return mm.used_memory;
}

/*----------------------------------------------------------------
 * Funciones de depuración
 *----------------------------------------------------------------*/

/* int_to_str - Convierte entero a string */
static void int_to_str(uint32_t num, char* buf) {
    if (num == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    char temp[20];
    int i = 0;

    while (num > 0) {
        temp[i++] = (num % 10) + '0';
        num /= 10;
    }

    /* Invertir */
    int len = i;
    for (int j = 0; j < len; j++) {
        buf[j] = temp[len - j - 1];
    }
    buf[len] = '\0';
}
/* mm_dump_info - Muestra información de memoria */
void mm_dump_info(void) {
    char buffer[32];

    printk("=== Memory Manager ===", cursor_y++, WHITE);

    printk("Total: ", cursor_y++, WHITE);
    int_to_str(mm_get_total() / (1024*1024), buffer);
    printk(buffer, cursor_y++, WHITE);
    printk(" MB", cursor_y++, WHITE);

    printk("Free: ", cursor_y++, WHITE);
    int_to_str(mm_get_free() / (1024*1024), buffer);
    printk(buffer, cursor_y++, WHITE);
    printk(" MB", cursor_y++, WHITE);

    printk("Used: ", cursor_y++, WHITE);
    int_to_str(mm_get_used() / (1024*1024), buffer);
    printk(buffer, cursor_y++, WHITE);
    printk(" MB", cursor_y++, WHITE);

    printk("Free pages: ", cursor_y++, WHITE);
    int_to_str(mm.free_pages, buffer);
    printk(buffer, cursor_y++, WHITE);
    printk("=====================", cursor_y++, WHITE);
}
