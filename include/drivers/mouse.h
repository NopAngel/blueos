#ifndef _MOUSE_H
#define _MOUSE_H

void mouse_wait(uint8_t type);
void mouse_write(uint8_t data);
uint8_t mouse_read();
void mouse_init();
void mouse_handler(struct trap_frame *tf);

#endif