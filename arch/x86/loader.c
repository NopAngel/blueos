#include <fs/fat16.h>
#include <lib/string.h>
#include <stddef.h>
#include <kernel/printk.h>
#include <kernel/colors.h>

#define LOAD_ADDRESS 0x002f7000
#define EXEC_ADDRESS 0x000a0000

typedef void (*entry_point_t)(void);

void load_and_run_init() {
    uint8_t* file_data = fat16_read_file("INIT.BIN");

    if (file_data != NULL) {
        memcpy((void*)EXEC_ADDRESS, (void*)file_data, 512);

        entry_point_t start_program = (entry_point_t)EXEC_ADDRESS;

        start_program();
    } else {
        printk(RED, "Error in INIT.BIN\n");
    }
}
