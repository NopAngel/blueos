#ifndef BITMAP_H
#define BITMAP_H

#define BITMAP_SIZE 8
void draw_num_bitmaps(short index, short x, short y, char color);
void draw_alpha_bitmaps(short index, short x, short y, char color);
void draw_char(short x, short y, char color, char ch);
void draw_string(short x, short y, char color, char *str);

#endif