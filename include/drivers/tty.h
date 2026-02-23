#ifndef _BLUEOS_TTY_H
#define _BLUEOS_TTY_H

#include <stddef.h>

#define TTY_BUFFER_SIZE 1024

struct tty_struct {
    int index;                
    char input_buffer[TTY_BUFFER_SIZE];
    size_t head;
    size_t tail;
    size_t count;


    int echo;                      
    int raw_mode;                  
    void (*write)(const char *buf, size_t n);
};

void tty_init();
void tty_receive_char(struct tty_struct *tty, char c);
int tty_read(struct tty_struct *tty, char *buf, size_t n);

#endif