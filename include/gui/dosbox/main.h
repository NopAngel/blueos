#ifndef KERNEL_H
#define KERNEL_H

#include "./types.h"

#define NULL 0

#define VGA_ADDRESS 0xB8000
#define BUFSIZE 2200


#define BOX_MAX_WIDTH 78
#define BOX_MAX_HEIGHT 23

#define BOX_SINGLELINE 1
#define BOX_DOUBLELINE 2

enum vga_color {
    VGA_BLACK,
    VGA_BLUE,
    VGA_GREEN,
    VGA_CYAN,
    VGA_RED,
    VGA_MAGENTA,
    VGA_BROWN,
    VGA_GREY,
    VGA_DARK_GREY,
    VGA_BRIGHT_BLUE,
    VGA_BRIGHT_GREEN,
    VGA_BRIGHT_CYAN,
    VGA_BRIGHT_RED,
    VGA_BRIGHT_MAGENTA,
    VGA_YELLOW,
    VGA_WHITE,
};

short vga_entry(unsigned char ch, char fore_color, char back_color);
void clear_vga_buffer(short **buffer, char fore_color, char back_color);

void init_vga(char fore_color, char back_color);
void print_new_line();
void print_char(char ch);
void print_string(char *str);
void print_color_string(char *str, char fore_color, char back_color);
void print_int(int num);

short get_box_draw_char(char chn, char fore_color, char back_color);
void gotoxy(short x, short y);
void draw_generic_box(short x, short y, 
                      short width, short height,
                      char fore_color, char back_color,
                      char topleft_ch,
                      char topbottom_ch,
                      char topright_ch,
                      char leftrightside_ch,
                      char bottomleft_ch,
                      char bottomright_ch);

void draw_box(char boxtype, 
              short x, short y, 
              short width, short height,
              char fore_color, char back_color);
void fill_box(char ch, short x, short y, short width, short height, char color);
void create_dosbox_ui();


#endif
