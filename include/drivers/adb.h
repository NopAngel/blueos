#ifndef ADB_H
#define ADB_H

#include <stdint.h>

#define ADB_ADDR_KEYBOARD  0x02
#define ADB_ADDR_MOUSE     0x03

#define ADB_CMD_TALK       0x0C 
#define ADB_CMD_LISTEN     0x08 
#define ADB_CMD_FLUSH      0x01 

struct adb_packet {
    uint8_t address;
    uint8_t register_num;
    uint8_t data[8];
    uint8_t len;
};

void mac_adb_init();
int  mac_adb_talk(uint8_t addr, uint8_t reg, uint8_t *buffer);
void mac_adb_handler(); 

#endif