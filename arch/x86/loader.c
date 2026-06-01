#include <lib/string.h>
#include <stddef.h>
#include <kernel/printk.h>
#include <kernel/colors.h>

extern void jump_to_user(void* address);

/* Movido a 1MB+ para evitar el agujero de VRAM (0xA0000) */
#define EXEC_ADDRESS 0x00400000

typedef void (*entry_point_t)(void);

void load_and_run_init() {
    // Usamos vfs_read que es la interfaz genérica para ramfs/vfs
    extern void* vfs_read(const char* filename);
    
    uint8_t* file_data = (uint8_t*)vfs_read("HELLO.BIN");

    if (file_data != NULL) {
        memcpy((void*)EXEC_ADDRESS, (void*)file_data, 4096);
        jump_to_user((void*)EXEC_ADDRESS);
    } else {
        printk("Error: Could not load HELLO.BIN\n");
    }
}
