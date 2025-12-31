#ifndef BITMAP_H
#define BITMAP_H

#define BITMAP_SIZE 8

extern int bitmaps_0_9[10][BITMAP_SIZE];
extern int bitmaps_A_Z[26][BITMAP_SIZE];

void draw_num_bitmaps(short index, short x, short y, char color);
void draw_alpha_bitmaps(short index, short x, short y, char color);
void draw_char(short x, short y, char color, char ch);
void draw_string(short x, short y, char color, char *str);
void draw_custom(short x, short y, char color, unsigned char bitmap[8]);

#endif