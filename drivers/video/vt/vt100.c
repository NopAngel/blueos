#include <kernel/ports.h>
#include <stdint.h>

#define VIDEO_ADDRESS 0xb8000
#define MAX_ROWS 25
#define MAX_COLS 80

#define TEXT_MAX_ROWS 25
#define TEXT_MAX_COLS 80

#define GUI_MAX_ROWS 60
#define GUI_MAX_COLS 125
/* ANSI State Machine Definitions */
typedef enum { STATE_NORMAL, STATE_ESC, STATE_CSI } ansi_state_t;

/* Variables globales compartidas del sistema de consolas */
extern int cursor_x, cursor_y;
static unsigned char color = 0x0F;
static ansi_state_t current_state = STATE_NORMAL;
static char param_buffer[32];
static int param_ptr = 0;

/* Control de la interfaz gráfica */
extern int g_gui_enabled;
extern void fb_draw_char(uint32_t x_start, uint32_t y_start, char c, uint32_t color);
extern void fb_draw_rect(uint32_t x_start, uint32_t y_start, uint32_t width, uint32_t height, uint32_t color);
extern void fb_scroll_up(uint32_t pixels, uint32_t bg_color);

#define GRAPHIC_BG_COLOR   0x000000  /* Color de fondo por defecto (Blanco o Pastel) */
#define FONT_SCALE_X       8
#define FONT_SCALE_Y       12        /* 8px de fuente + 4px de espaciado vertical */
#define GRAPHIC_MARGIN_X   10
#define GRAPHIC_MARGIN_Y   10

/* Mapeo ANSI de color de 24 bits para el Framebuffer gráfico */
static uint32_t ansi_to_rgb[] = {
    0x000000, // 30: Negro
    0xFF5555, // 31: Rojo
    0x55FF55, // 32: Verde
    0xFFFF55, // 33: Amarillo
    0x5555FF, // 34: Azul
    0xFF55FF, // 35: Magenta
    0x55FFFF, // 36: Cian
    0xFFFFFF  /* 37: Blanco */
};

/* ANSI Color Mapping (30-37) to VGA Text Colors */
static uint8_t ansi_to_vga[] = { 0, 4, 2, 6, 1, 5, 3, 7 };

/* Color RGB gráfico actual basado en el estado ANSI */
static uint32_t current_rgb_color = 0xFFFFFF;

static void set_cursor(int x, int y) {
  if (g_gui_enabled) return; /* En modo gráfico no tocamos los puertos VGA de texto */
  
  unsigned short pos = y * MAX_COLS + x;
  outb(0x3D4, 0x0F);
  outb(0x3D5, (unsigned char)(pos & 0xFF));
  outb(0x3D4, 0x0E);
  outb(0x3D5, (unsigned char)((pos >> 8) & 0xFF));
}

static void scroll(void) {
  if (g_gui_enabled) {
    /* Scroll gráfico: Desplazamos la pantalla hacia arriba */
    fb_scroll_up(FONT_SCALE_Y, GRAPHIC_BG_COLOR);
  } else {
    /* Scroll clásico en modo texto VGA */
    unsigned short *video_memory = (unsigned short *)VIDEO_ADDRESS;
    for (int i = 0; i < (MAX_ROWS - 1) * MAX_COLS; i++) {
      video_memory[i] = video_memory[i + MAX_COLS];
    }
    for (int i = (MAX_ROWS - 1) * MAX_COLS; i < MAX_ROWS * MAX_COLS; i++) {
      video_memory[i] = ' ' | (color << 8);
    }
  }
  cursor_y = MAX_ROWS - 1;
}

void vt100_init(void) {
  if (g_gui_enabled) {
    /* Limpieza completa de la GUI */
    fb_draw_rect(0, 0, 1024, 768, GRAPHIC_BG_COLOR);
  } else {
    unsigned short *video_memory = (unsigned short *)VIDEO_ADDRESS;
    for (int i = 0; i < MAX_ROWS * MAX_COLS; i++) {
      video_memory[i] = ' ' | (color << 8);
    }
  }
  cursor_x = 0;
  cursor_y = 0;
  set_cursor(cursor_x, cursor_y);
}

void vt100_set_color(uint8_t new_color) { color = new_color; }

static void handle_ansi_csi(char command) {
  param_buffer[param_ptr] = '\0';
  int params[4] = {0, 0, 0, 0};
  int p_idx = 0;
  char *ptr = param_buffer;

  while (*ptr && p_idx < 4) {
    if (*ptr >= '0' && *ptr <= '9') {
      params[p_idx] = params[p_idx] * 10 + (*ptr - '0');
    } else if (*ptr == ';') {
      p_idx++;
    }
    ptr++;
  }

  int val = params[0];

  switch (command) {
  case 'm': // Cambios de color ANSI
    for (int i = 0; i <= p_idx; i++) {
      int v = params[i];
      if (v == 0) {
        color = 0x0F;
        current_rgb_color = 0xFFFFFF; /* Reset a Negro gráfico */
      } else if (v >= 30 && v <= 37) {
        color = (color & 0x08) | ansi_to_vga[v - 30];
        current_rgb_color = ansi_to_rgb[v - 30]; /* Actualizamos el color gráfico */
      } else if (v >= 90 && v <= 97) {
        color = 0x08 | ansi_to_vga[v - 90];
        current_rgb_color = ansi_to_rgb[v - 90];
      }
    }
    break;

  case 'A': cursor_y = (cursor_y > (val ? val : 1)) ? cursor_y - (val ? val : 1) : 0; break;
  case 'B': cursor_y = (cursor_y + (val ? val : 1) < MAX_ROWS) ? cursor_y + (val ? val : 1) : MAX_ROWS - 1; break;
  case 'C': cursor_x = (cursor_x + (val ? val : 1) < MAX_COLS) ? cursor_x + (val ? val : 1) : MAX_COLS - 1; break;
  case 'D': cursor_x = (cursor_x > (val ? val : 1)) ? cursor_x - (val ? val : 1) : 0; break;
  
  case 'H':
  case 'f':
    if (p_idx >= 1) {
      cursor_y = (params[0] > 0) ? params[0] - 1 : 0;
      cursor_x = (params[1] > 0) ? params[1] - 1 : 0;
    } else {
      cursor_x = 0; cursor_y = 0;
    }
    break;

  case 'J': if (val == 2) vt100_init(); break;
  }
}

void vt100_putc(char c) {
  int max_cols = g_gui_enabled ? GUI_MAX_COLS : TEXT_MAX_COLS;
  int max_rows = g_gui_enabled ? GUI_MAX_ROWS : TEXT_MAX_ROWS;

  switch (current_state) {
  case STATE_NORMAL:
    if (c == 27) {
      current_state = STATE_ESC;
    } else if (c == '\n') {
      cursor_x = 0;
      cursor_y++;
    } else if (c == '\r') {
      cursor_x = 0;
    } else if (c == '\t') {
      cursor_x = (cursor_x + 8) & ~(7);
    } else if (c == '\b') {
      if (cursor_x > 0) {
        cursor_x--;
        if (g_gui_enabled) {
          uint32_t gx = GRAPHIC_MARGIN_X + (cursor_x * FONT_SCALE_X);
          uint32_t gy = GRAPHIC_MARGIN_Y + (cursor_y * FONT_SCALE_Y);
          fb_draw_rect(gx, gy, FONT_SCALE_X, FONT_SCALE_Y, GRAPHIC_BG_COLOR);
        }
      }
    } else {
      if (g_gui_enabled) {
        uint32_t gx = GRAPHIC_MARGIN_X + (cursor_x * FONT_SCALE_X);
        uint32_t gy = GRAPHIC_MARGIN_Y + (cursor_y * FONT_SCALE_Y);
        fb_draw_char(gx, gy, c, current_rgb_color);
      } else {
        unsigned short *video_memory = (unsigned short *)VIDEO_ADDRESS;
        video_memory[cursor_y * TEXT_MAX_COLS + cursor_x] = c | (color << 8);
      }
      cursor_x++;
    }
    break;

  case STATE_ESC:
    if (c == '[') { current_state = STATE_CSI; param_ptr = 0; } 
    else { current_state = STATE_NORMAL; }
    break;

  case STATE_CSI:
    if ((c >= '0' && c <= '9') || c == ';') {
      if (param_ptr < 31) param_buffer[param_ptr++] = c;
    } else {
      handle_ansi_csi(c);
      current_state = STATE_NORMAL;
    }
    break;
  }

  /* --- CONTROL DE DESBORDAMIENTO DINÁMICO --- */
  if (cursor_x >= max_cols) {
    cursor_x = 0;
    cursor_y++;
  }
  if (cursor_y >= max_rows) {
    scroll();
    cursor_y = max_rows - 1; /* Mantenemos el cursor en la última línea útil */
  }

  if (current_state == STATE_NORMAL) {
    set_cursor(cursor_x, cursor_y);
  }
}

void vt100_puts(const char *s) {
  while (*s) {
    vt100_putc(*s++);
  }
}