#include <include/gui/vga.h>

// a global VGA buffer
static char back_buffer[VGA_MAX];
static char* g_vga_buffer;

static inline unsigned char custom_inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static inline void custom_outb(unsigned short port, unsigned char data) {
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}
/*
See Intel® OpenSource HD Graphics PRM pdf file
for following defined data for each vga register
and its explaination
*/
static char seq_data[5] = {0x03, 0x01, 0x0F, 0x00, 0x0E};
static char crtc_data[25] = {0x5F, 0x4F, 0x50, 0x82,
                              0x54, 0x80, 0xBF, 0x1F,
                              0x00, 0x41, 0x00, 0x00,
                              0x00, 0x00, 0x00, 0x00,
                              0x9C, 0x0E, 0x8F, 0x28,
                              0x40, 0x96, 0xB9, 0xA3,
                              0xFF};

static char gc_data[9] = {0x00, 0x00, 0x00, 0x00,
                          0x00, 0x40, 0x05, 0x0F,
                          0xFF};

static char ac_data[21] = {0x00, 0x01, 0x02, 0x03,
                            0x04, 0x05, 0x06, 0x07,
                            0x08, 0x09, 0x0A, 0x0B,
                            0x0C, 0x0D, 0x0E, 0x0F,
                            0x41, 0x00, 0x0F, 0x00,
                            0x00};

void set_misc()
{
  custom_outb(VGA_MISC_WRITE, 0x63);
}

void set_seq()
{
  // write sequence data to index of 0-4
  for(char index = 0; index < 5; index++){
    // first set index
    custom_outb(VGA_SEQ_INDEX, index);
    // write data at that index
    custom_outb(VGA_SEQ_DATA, seq_data[index]);
  }
}

void set_crtc()
{
  // write crtc data to index of 0-24
  for(char index = 0; index < 25; index++){
    custom_outb(VGA_CRTC_INDEX, index);
	  custom_outb(VGA_CRTC_DATA, crtc_data[index]);
  }
}

void set_gc()
{
  // write gc data to index of 0-8
  for(char index = 0; index < 9; index++){
    custom_outb(VGA_GC_INDEX, index);
    custom_outb(VGA_GC_DATA, gc_data[index]);
  }
}

void set_ac()
{
  char d;
  // write ac data to index of 0-20
  for(char index = 0; index < 21; index++){
    custom_outb(VGA_AC_INDEX, index);
    custom_outb(VGA_AC_WRITE, ac_data[index]);
  }
  d = custom_inb(VGA_INSTAT_READ);
  custom_outb(VGA_AC_INDEX, d | 0x20);
}

void vga_clear_screen()
{
  for(int index = 0; index < VGA_MAX; index++)
    g_vga_buffer[index] = 0;
}

void clear_color(char color)
{
  for(int index = 0; index < VGA_MAX; index++)
    g_vga_buffer[index] = color;
}

void init_vga_fnc()
{
  set_misc();
  set_seq();
  set_crtc();
  set_gc();
  set_ac();

  g_vga_buffer = (char*)VGA_ADDRESS;

  vga_clear_screen();
}

void putpixel(short x, short y, char color) {
    if(x >= 0 && x < 320 && y >= 0 && y < 200) {
        back_buffer[320 * y + x] = color;
    }
}

void draw_line(short x1, short y1, short x2, short y2, char color) {
    int dx =  abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
    int err = dx + dy, e2;

    for (;;) {
        putpixel(x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x1 += sx; }
        if (e2 <= dx) { err += dx; y1 += sy; }
    }
}

int abs(int v) { return v < 0 ? -v : v; }

void vga_update() {
    __asm__ volatile (
        "movl %0, %%ecx\n"      // VGA_MAX / 4 (para copiar de 4 en 4 bytes)
        "movl %1, %%esi\n"      
        "movl %2, %%edi\n"      
        "rep movsl\n"           
        :
        : "g"(VGA_MAX / 4), "r"(back_buffer), "r"(g_vga_buffer)
        : "ecx", "esi", "edi", "memory"
    );
}

void draw_rect(short x, short y, short width, short height, char color)
{
  draw_line(x, y, x, y + height, color);
  draw_line(x, y, x + width, y, color);
  draw_line(x + width, y, x + width, y + height, color);
  draw_line(x, y + height, x + width, y + height, color);
}

void fill_screen(char color)
{
    for(int i = 0; i < VGA_MAX; i++)
        g_vga_buffer[i] = color;
}

void fill_rect(short x, short y, short width, short height, char color) {
    for (int i = 0; i < height; i++) {
     
        char* dest = &back_buffer[320 * (y + i) + x];
        for (int j = 0; j < width; j++) {
            dest[j] = color; 
        }
    }
}

void draw_bresenham_circle(int xc, int yc, int x, int y, char color)
{
  putpixel(xc+x, yc+y, color);
  putpixel(xc-x, yc+y, color);
  putpixel(xc+x, yc-y, color);
  putpixel(xc-x, yc-y, color);
  putpixel(xc+y, yc+x, color);
  putpixel(xc-y, yc+x, color);
  putpixel(xc+y, yc-x, color);
  putpixel(xc-y, yc-x, color);
}

void draw_circle(short x, short y, short radius, char color)
{
  int x2 = 0, y2 = radius;
  int d = 3 - 2 * radius;
  draw_bresenham_circle(x, y, x2, y2, color);
  while(y2 >= x2){
    x2++;
    if(d > 0){
      y2--;
      d = d + 4 * (x2 - y2) + 10;
    }else{
      d = d + 4 * x2 + 6;
    }
    draw_bresenham_circle(x, y, x2, y2, color);
  }
}

void draw_diamond(short x, short y, short radius, char color)
{
  short x2 = 0, y2 = radius;
  short d = 3 - 2 * radius;
  draw_bresenham_circle(x, y, x2, y2, color);
  while(y2 >= x2){
    x2++;
    if(d > 0){
      y2--;
      d = d + 4 * (x2 - y2) + 10;
    }else{
      d = d + 4 * x2 + 6;
    }
    draw_bresenham_circle(x, y, x2, y2, color);
  }
}
