
#include <lib/string.h>
#include <kernel/printk.h>
/*
static event_node_t event_table[MAX_EVENTS];
static int event_count = 0;

void eventfs_init() {
    event_count = 0;
    memset(event_table, 0, sizeof(event_table));
}

int eventfs_register(char* name, uint32_t (*callback)()) {
    if (event_count >= MAX_EVENTS) return -1;
    
    strcpy(event_table[event_count].name, name);
    event_table[event_count].read_callback = callback;
    event_count++;
    return 0;
}

uint32_t eventfs_read(char* name) {
    for (int i = 0; i < event_count; i++) {
        if (strcmp(event_table[i].name, name) == 0) {
            return event_table[i].read_callback();
        }
    }
    return 0xFFFFFFFF; // Error: event not found
}*/