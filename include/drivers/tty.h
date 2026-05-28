#ifndef TTY_H
#define TTY_H

#include <stdint.h>

#define VIDEO_SIZE (80 * 25 * 2)

#ifndef MAX_TTYS
#define MAX_TTYS 7
#endif

#ifndef TTY_BUFFER_SIZE
#define TTY_BUFFER_SIZE 2048
#endif

#ifndef CTRL
#define CTRL(c) ((c) & 0x1F)
#endif

#ifndef ICANON
#define ICANON  0000002
#define ECHO    0000010
#define ISIG    0000001
#define VINTR   0
#define VERASE  1
#define VKILL   2
#define VEOF    3
#endif

typedef struct {
    uint32_t c_iflag;
    uint32_t c_oflag;
    uint32_t c_lflag;
    uint8_t  c_cc[20];
} struct_termios;

typedef struct {
    int id;
    char buffer[TTY_BUFFER_SIZE];
    uint32_t head, tail, count;
    
    uint16_t screen_buffer[VIDEO_SIZE / 2]; 
    int saved_cursor_x;
    int saved_cursor_y;
    
    struct_termios conf;
    void (*hw_write)(char c);
} tty_t;

/* Declaraciones para que auth.c las vea */
tty_t* get_current_tty_struct();
void restore_screen_from_buffer(tty_t *tty);
void save_screen_to_buffer(tty_t *tty);
void tty_switch(int id);
void tty_init(void (*hw_write)(char));

#endif




