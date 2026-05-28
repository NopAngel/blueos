#ifndef TC_H
#define TC_H

#include <stdint.h>

#define TC_ECHO     (1 << 0)  
#define TC_ICANON   (1 << 1)  
#define TC_ISIG     (1 << 2)  

struct tc_settings {
    uint32_t flags;
    char erase_char;  
    char kill_char;   
};

void tc_init();
void tc_handle_input(char c);
void tc_set_color(const char *ansi_color);

#endif