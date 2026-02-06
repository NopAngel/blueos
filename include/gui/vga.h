#ifndef VGA_H
#define VGA_H


#define VGA_ADDRESS 0xA0000
#define VGA_MAX 0xF9FF
#define VGA_MAX_WIDTH 320
#define VGA_MAX_HEIGHT 200

enum gui_vga_color {
  GUI_BLACK,
  GUI_BLUE,
  GUI_GREEN,
  GUI_CYAN,
  GUI_RED,
  GUI_MAGENTA,
  GUI_BROWN,
  GUI_GREY,
  GUI_DARK_GREY,
  GUI_BRIGHT_BLUE,
  GUI_BRIGHT_GREEN,
  GUI_BRIGHT_CYAN,
  GUI_BRIGHT_RED,
  GUI_BRIGHT_MAGENTA,
  GUI_YELLOW,
  GUI_WHITE,
};

/* Attribute Controller Registers */
#define	VGA_AC_INDEX 0x3C0
#define	VGA_AC_READ 0x3C1
#define	VGA_AC_WRITE 0x3C0

/*
Miscellaneous Output
*/
#define	VGA_MISC_READ 0x3CC
#define	VGA_MISC_WRITE 0x3C2

/* Sequencer Registers */
#define VGA_SEQ_INDEX 0x3C4
#define VGA_SEQ_DATA 0x3C5

/* VGA Color Palette Registers */
#define	VGA_DAC_READ_INDEX 0x3C7
#define	VGA_DAC_WRITE_INDEX 0x3C8
#define	VGA_DAC_DATA 0x3C9

/* Graphics Controller Registers */
#define VGA_GC_INDEX 0x3CE
#define VGA_GC_DATA 0x3CF

/* CRT Controller Registers */
#define VGA_CRTC_INDEX 0x3D4
#define VGA_CRTC_DATA 0x3D5

/* General Control and Status Registers */
#define	VGA_INSTAT_READ 0x3DA

void init_vga_fnc();
void vga_clear_screen();
void clear_color(char color);
void putpixel(short x, short y, char color);
void draw_line(short x1, short y1, short x2, short y2, char color);
int abs(int v);
void vga_update();
void draw_rect(short x, short y, short width, short height, char color);
void fill_screen(char color);
void fill_rect(short x, short y, short width, short height, char color);
void draw_circle(short x, short y, short radius, char color);
void draw_diamond(short x, short y, short radius, char color);

#endif