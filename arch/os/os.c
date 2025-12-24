#include "../../include/gui/vga.h"
#include "../../include/gui/bitmap.h"

int cursor_x;
int cursor_y;

extern int bitmaps_0_9[10][8];
extern int bitmaps_A_Z[26][8];

// ========== DEFINICIONES ==========
#define MOUSE_DATA_PORT 0x60
#define PS2_CMD_PORT 0x64

// Bits del status byte
#define LEFT_BUTTON   0x01
#define RIGHT_BUTTON  0x02
#define ALWAYS_1      0x08
#define X_SIGN        0x10
#define Y_SIGN        0x20

// ========== ESTRUCTURAS ==========
typedef struct {
    int x, y;
    int width, height;
    char title[32];
    char visible;
    char dragging;
    int content_id; // 0=terminal, 1=calc, 2=paint
} Window;

typedef struct {
    int x, y;
    char text[32];
    char icon; // 0=terminal, 1=calc, 2=paint, 3=folder
    char pressed;
} TaskbarButton;

// ========== VARIABLES GLOBALES ==========
int mouse_x = 160;
int mouse_y = 100;
char mouse_left_pressed = 0;
char mouse_right_pressed = 0;
char mouse_enabled = 0;

// Ventanas
Window windows[3] = {
    {50, 50, 200, 150, "BlueSH", 0, 0, 0},
    {100, 80, 180, 120, "BlueCalc", 0, 0, 1},
    {80, 60, 220, 180, "PAINT", 0, 0, 2}
};

// Botones de la barra de tareas
TaskbarButton taskbar_buttons[4] = {
    {5, 185, "TERM", 0, 0},
    {85, 185, "CALC", 1, 0},
    {145, 185, "PAINT", 2, 0},
    {205, 185, "FILES", 3, 0}
};

char start_menu_open = 0;
int start_menu_x = 2;
int start_menu_y = 170;

// Back buffer
#define VGA_WIDTH 320
#define VGA_HEIGHT 200
#define VGA_SIZE (VGA_WIDTH * VGA_HEIGHT)
char back_buffer[VGA_SIZE];
char* front_buffer = (char*)VGA_ADDRESS;

// Colores Windows 98
#define WIN_BLUE 1
#define WIN_DARK_BLUE 9
#define WIN_GREY 7
#define WIN_DARK_GREY 8
#define WIN_LIGHT_GREY 15
#define WIN_BLACK 0
#define WIN_WHITE 15
#define WIN_GREEN 10
#define WIN_RED 4
#define WIN_YELLOW 14

// ========== FUNCIONES DEL MOUSE (DEL CÓDIGO QUE FUNCIONA) ==========
static inline unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static inline void outb(unsigned short port, unsigned char data) {
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

void delay(int count) {
    volatile int i;
    for(i = 0; i < count; i++);
}

void mouse_wait(char type) {
    unsigned int timeout = 100000;
    if(type == 0) {
        while(timeout--) {
            if((inb(PS2_CMD_PORT) & 1) == 1) return;
        }
    } else {
        while(timeout--) {
            if((inb(PS2_CMD_PORT) & 2) == 0) return;
        }
    }
}

void mouse_write(unsigned char data) {
    mouse_wait(1);
    outb(PS2_CMD_PORT, 0xD4);
    mouse_wait(1);
    outb(MOUSE_DATA_PORT, data);
}

unsigned char mouse_read() {
    mouse_wait(0);
    return inb(MOUSE_DATA_PORT);
}

void mouse_init() {
    unsigned char status;
    
    mouse_wait(1);
    outb(PS2_CMD_PORT, 0xA8);
    
    // Habilitar interrupciones
    mouse_wait(1);
    outb(PS2_CMD_PORT, 0x20);
    mouse_wait(0);
    status = inb(MOUSE_DATA_PORT);
    status |= 0x02;
    status &= ~0x20;
    mouse_wait(1);
    outb(PS2_CMD_PORT, 0x60);
    mouse_wait(1);
    outb(MOUSE_DATA_PORT, status);
    
    mouse_write(0xF6);
    mouse_read();
    mouse_write(0xF4);
    mouse_read();
    
    mouse_enabled = 1;
}

void mouse_handler() {
    static unsigned char mouse_cycle = 0;
    static unsigned char mouse_byte[3];
    
    switch(mouse_cycle) {
        case 0:
            mouse_byte[0] = mouse_read();
            if(!(mouse_byte[0] & ALWAYS_1)) {
                mouse_cycle = 0;
                return;
            }
            mouse_left_pressed = (mouse_byte[0] & LEFT_BUTTON) ? 1 : 0;
            mouse_right_pressed = (mouse_byte[0] & RIGHT_BUTTON) ? 1 : 0;
            mouse_cycle++;
            break;
            
        case 1:
            mouse_byte[1] = mouse_read();
            mouse_cycle++;
            break;
            
        case 2:
            mouse_byte[2] = mouse_read();
            
            // Procesar movimiento
            char delta_x = mouse_byte[1];
            char delta_y = mouse_byte[2];
            
            int dx = delta_x;
            int dy = delta_y;
            
            if(mouse_byte[0] & X_SIGN) dx = dx | 0xFFFFFF00;
            if(mouse_byte[0] & Y_SIGN) dy = dy | 0xFFFFFF00;
            
            mouse_x += dx;
            mouse_y -= dy;
            
            if(mouse_x < 0) mouse_x = 0;
            if(mouse_x >= 320) mouse_x = 319;
            if(mouse_y < 0) mouse_y = 0;
            if(mouse_y >= 200) mouse_y = 199;
            
            // Procesar clics
            static char last_left = 0;
            
            if(mouse_left_pressed && !last_left) {
                // Click presionado
                
                // Verificar ventanas
                for(int i = 0; i < 3; i++) {
                    if(windows[i].visible) {
                        if(mouse_x >= windows[i].x && mouse_x <= windows[i].x + windows[i].width &&
                           mouse_y >= windows[i].y && mouse_y <= windows[i].y + 20) {
                            windows[i].dragging = 1;
                            // Traer al frente
                            for(int j = 0; j < 3; j++) {
                                if(j != i) windows[j].dragging = 0;
                            }
                            break;
                        }
                    }
                }
                
                // Verificar botones de la barra
                for(int i = 0; i < 4; i++) {
                    if(mouse_x >= taskbar_buttons[i].x && mouse_x <= taskbar_buttons[i].x + 70 &&
                       mouse_y >= taskbar_buttons[i].y && mouse_y <= taskbar_buttons[i].y + 15) {
                        taskbar_buttons[i].pressed = 1;
                        if(i < 3) {
                            windows[i].visible = !windows[i].visible;
                            if(windows[i].visible) windows[i].dragging = 0;
                        }
                    }
                }
                
                // Verificar botón inicio
                if(mouse_x >= 2 && mouse_x <= 52 &&
                   mouse_y >= 187 && mouse_y <= 198) {
                    start_menu_open = !start_menu_open;
                }
                
                // Verificar botones de cerrar
                for(int i = 0; i < 3; i++) {
                    if(windows[i].visible) {
                        if(mouse_x >= windows[i].x + windows[i].width - 20 &&
                           mouse_x <= windows[i].x + windows[i].width - 6 &&
                           mouse_y >= windows[i].y + 5 &&
                           mouse_y <= windows[i].y + 17) {
                            windows[i].visible = 0;
                        }
                    }
                }
                
            } else if(!mouse_left_pressed) {
                // Click liberado
                for(int i = 0; i < 3; i++) {
                    windows[i].dragging = 0;
                }
                for(int i = 0; i < 4; i++) {
                    taskbar_buttons[i].pressed = 0;
                }
            }
            last_left = mouse_left_pressed;
            
            // Mover ventanas arrastradas
            for(int i = 0; i < 3; i++) {
                if(windows[i].dragging) {
                    windows[i].x = mouse_x - (windows[i].width / 2);
                    windows[i].y = mouse_y - 10;
                    
                    if(windows[i].x < 0) windows[i].x = 0;
                    if(windows[i].x + windows[i].width > 320) windows[i].x = 320 - windows[i].width;
                    if(windows[i].y < 20) windows[i].y = 20;
                    if(windows[i].y + windows[i].height > 185) windows[i].y = 185 - windows[i].height;
                }
            }
            
            mouse_cycle = 0;
            break;
    }
}

// ========== FUNCIONES DE BACK BUFFER ==========
void swap_buffers() {
    for(int i = 0; i < VGA_SIZE; i++) {
        front_buffer[i] = back_buffer[i];
    }
}

void fill_backbuffer(char color) {
    for(int i = 0; i < VGA_SIZE; i++) {
        back_buffer[i] = color;
    }
}

void putpixel_backbuffer(short x, short y, char color) {
    if(x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT) {
        back_buffer[y * VGA_WIDTH + x] = color;
    }
}

void draw_line_backbuffer(short x1, short y1, short x2, short y2, char color) {
    if(y1 == y2) {
        if(x1 > x2) { short temp = x1; x1 = x2; x2 = temp; }
        for(short i = x1; i <= x2; i++)
            putpixel_backbuffer(i, y1, color);
        return;
    }
    if(x1 == x2) {
        if(y1 > y2) { short temp = y1; y1 = y2; y2 = temp; }
        for(short i = y1; i <= y2; i++)
            putpixel_backbuffer(x1, i, color);
        return;
    }
}

void draw_rect_backbuffer(short x, short y, short width, short height, char color) {
    draw_line_backbuffer(x, y, x + width, y, color);
    draw_line_backbuffer(x + width, y, x + width, y + height, color);
    draw_line_backbuffer(x, y + height, x + width, y + height, color);
    draw_line_backbuffer(x, y, x, y + height, color);
}

void draw_char_backbuffer(short x, short y, char color, char ch) {
    if(ch >= '0' && ch <= '9'){
        // Dibujar número
        int index = ch - '0';
        for(int row = 0; row < BITMAP_SIZE; row++){
            int byte = bitmaps_0_9[index][row];
            for(int col = 0; col < BITMAP_SIZE; col++){
                if(byte & (1 << (7 - col))) { // Bit de izquierda a derecha
                    putpixel_backbuffer(x + col, y + row, color);
                }
            }
        }
    }
    else if(ch >= 'A' && ch <= 'Z'){
        // Dibujar letra mayúscula
        int index = ch - 'A';
        for(int row = 0; row < BITMAP_SIZE; row++){
            int byte = bitmaps_A_Z[index][row];
            for(int col = 0; col < BITMAP_SIZE; col++){
                if(byte & (1 << (7 - col))) { // Bit de izquierda a derecha
                    putpixel_backbuffer(x + col, y + row, color);
                }
            }
        }
    }
    else if(ch >= 'a' && ch <= 'z'){
        // Dibujar letra minúscula (convertir a mayúscula)
        int index = ch - 'a';
        for(int row = 0; row < BITMAP_SIZE; row++){
            int byte = bitmaps_A_Z[index][row];
            for(int col = 0; col < BITMAP_SIZE; col++){
                if(byte & (1 << (7 - col))) { // Bit de izquierda a derecha
                    putpixel_backbuffer(x + col, y + row, color);
                }
            }
        }
    }
    else if(ch == ' '){
        // Espacio - no dibujar nada
        return;
    }
    else {
        // Caracteres especiales simples
        switch(ch){
            case '.':
                // Punto
                putpixel_backbuffer(x + 3, y + 7, color);
                putpixel_backbuffer(x + 4, y + 7, color);
                break;
                
            case ',':
                // Coma
                putpixel_backbuffer(x + 3, y + 6, color);
                putpixel_backbuffer(x + 4, y + 6, color);
                putpixel_backbuffer(x + 4, y + 7, color);
                break;
                
            case '!':
                // Exclamación
                for(int i = 0; i < 5; i++)
                    putpixel_backbuffer(x + 4, y + i, color);
                putpixel_backbuffer(x + 4, y + 7, color);
                break;
                
            case '?':
                // Signo de interrogación
                putpixel_backbuffer(x + 2, y + 0, color);
                putpixel_backbuffer(x + 3, y + 0, color);
                putpixel_backbuffer(x + 5, y + 0, color);
                putpixel_backbuffer(x + 6, y + 0, color);
                
                putpixel_backbuffer(x + 1, y + 1, color);
                putpixel_backbuffer(x + 4, y + 1, color);
                putpixel_backbuffer(x + 7, y + 1, color);
                
                for(int i = 2; i < 4; i++)
                    putpixel_backbuffer(x + 7, y + i, color);
                    
                putpixel_backbuffer(x + 4, y + 4, color);
                putpixel_backbuffer(x + 4, y + 6, color);
                break;
                
            case ':':
                // Dos puntos
                putpixel_backbuffer(x + 4, y + 2, color);
                putpixel_backbuffer(x + 4, y + 5, color);
                break;
                
            case ';':
                // Punto y coma
                putpixel_backbuffer(x + 4, y + 2, color);
                putpixel_backbuffer(x + 4, y + 5, color);
                putpixel_backbuffer(x + 3, y + 7, color);
                break;
                
            case '-':
                // Guión
                for(int i = 2; i < 6; i++)
                    putpixel_backbuffer(x + i, y + 4, color);
                break;
                
            case '_':
                // Guión bajo
                for(int i = 0; i < 8; i++)
                    putpixel_backbuffer(x + i, y + 7, color);
                break;
                
            case '(':
                // Paréntesis izquierdo
                putpixel_backbuffer(x + 5, y + 0, color);
                putpixel_backbuffer(x + 4, y + 1, color);
                putpixel_backbuffer(x + 3, y + 2, color);
                putpixel_backbuffer(x + 3, y + 3, color);
                putpixel_backbuffer(x + 3, y + 4, color);
                putpixel_backbuffer(x + 3, y + 5, color);
                putpixel_backbuffer(x + 4, y + 6, color);
                putpixel_backbuffer(x + 5, y + 7, color);
                break;
                
            case ')':
                // Paréntesis derecho
                putpixel_backbuffer(x + 2, y + 0, color);
                putpixel_backbuffer(x + 3, y + 1, color);
                putpixel_backbuffer(x + 4, y + 2, color);
                putpixel_backbuffer(x + 4, y + 3, color);
                putpixel_backbuffer(x + 4, y + 4, color);
                putpixel_backbuffer(x + 4, y + 5, color);
                putpixel_backbuffer(x + 3, y + 6, color);
                putpixel_backbuffer(x + 2, y + 7, color);
                break;
                
            case '[':
                // Corchete izquierdo
                for(int i = 0; i < 8; i++)
                    putpixel_backbuffer(x + 3, y + i, color);
                putpixel_backbuffer(x + 4, y + 0, color);
                putpixel_backbuffer(x + 4, y + 7, color);
                break;
                
            case ']':
                // Corchete derecho
                for(int i = 0; i < 8; i++)
                    putpixel_backbuffer(x + 4, y + i, color);
                putpixel_backbuffer(x + 3, y + 0, color);
                putpixel_backbuffer(x + 3, y + 7, color);
                break;
                
            case '<':
                // Menor que
                for(int i = 0; i < 3; i++) {
                    putpixel_backbuffer(x + (4-i), y + (i+2), color);
                    putpixel_backbuffer(x + (4-i), y + (6-i), color);
                }
                break;
                
            case '>':
                // Mayor que
                for(int i = 0; i < 3; i++) {
                    putpixel_backbuffer(x + (3+i), y + (i+2), color);
                    putpixel_backbuffer(x + (3+i), y + (6-i), color);
                }
                break;
                
            case '/':
                // Diagonal
                for(int i = 0; i < 8; i++)
                    putpixel_backbuffer(x + (7 - i), y + i, color);
                break;
                
            case '\\':
                // Diagonal inversa
                for(int i = 0; i < 8; i++)
                    putpixel_backbuffer(x + i, y + i, color);
                break;
                
            case '|':
                // Barra vertical
                for(int i = 0; i < 8; i++)
                    putpixel_backbuffer(x + 4, y + i, color);
                break;
                
            case '@':
                // Arroba
                draw_rect_backbuffer(x+2, y+2, 4, 4, color);
                putpixel_backbuffer(x+1, y+1, color);
                putpixel_backbuffer(x+6, y+1, color);
                putpixel_backbuffer(x+1, y+6, color);
                putpixel_backbuffer(x+6, y+6, color);
                break;
                
            case '#':
                // Gato/numeral
                for(int i = 1; i < 7; i++) {
                    putpixel_backbuffer(x + 2, y + i, color);
                    putpixel_backbuffer(x + 5, y + i, color);
                }
                for(int i = 2; i < 6; i++) {
                    putpixel_backbuffer(x + i, y + 2, color);
                    putpixel_backbuffer(x + i, y + 5, color);
                }
                break;
                
            case '$':
                // Dólar
                for(int i = 1; i < 7; i++)
                    putpixel_backbuffer(x + 4, y + i, color);
                putpixel_backbuffer(x + 3, y + 1, color);
                putpixel_backbuffer(x + 5, y + 1, color);
                putpixel_backbuffer(x + 3, y + 4, color);
                putpixel_backbuffer(x + 5, y + 4, color);
                putpixel_backbuffer(x + 2, y + 7, color);
                putpixel_backbuffer(x + 6, y + 7, color);
                break;
                
            case '%':
                // Porcentaje
                for(int i = 0; i < 2; i++) {
                    draw_rect_backbuffer(x + 1 + i, y + 1, 1, 1, color);
                    draw_rect_backbuffer(x + 5 - i, y + 5, 1, 1, color);
                }
                for(int i = 0; i < 8; i++)
                    putpixel_backbuffer(x + (7 - i), y + i, color);
                break;
                
            case '&':
                // Ampersand
                for(int i = 0; i < 3; i++) {
                    putpixel_backbuffer(x + 3 + i, y + 1, color);
                    putpixel_backbuffer(x + 1 + i, y + 4, color);
                }
                putpixel_backbuffer(x + 2, y + 2, color);
                putpixel_backbuffer(x + 2, y + 6, color);
                putpixel_backbuffer(x + 5, y + 5, color);
                putpixel_backbuffer(x + 6, y + 7, color);
                break;
                
            case '*':
                // Asterisco
                for(int i = 0; i < 3; i++) {
                    putpixel_backbuffer(x + (3+i), y + 2, color);
                    putpixel_backbuffer(x + (3+i), y + 6, color);
                    putpixel_backbuffer(x + (1+i), y + (4-i), color);
                    putpixel_backbuffer(x + (5-i), y + (4+i), color);
                }
                break;
                
            case '+':
                // Signo más
                for(int i = 2; i < 6; i++) {
                    putpixel_backbuffer(x + i, y + 4, color);
                    putpixel_backbuffer(x + 4, y + i, color);
                }
                break;
                
            case '=':
                // Signo igual
                for(int i = 2; i < 6; i++) {
                    putpixel_backbuffer(x + i, y + 3, color);
                    putpixel_backbuffer(x + i, y + 5, color);
                }
                break;
                
            default:
                // Carácter desconocido - dibujar un cuadrado
                draw_rect_backbuffer(x, y, 8, 8, color);
                break;
        }
    }
}

void draw_string_backbuffer(short x, short y, char color, const char *str) {
    int index = 0;
    short current_x = x;
    
    while(str[index] != '\0'){
        // Dibujar el carácter actual
        draw_char_backbuffer(current_x, y, color, str[index]);
        
        // Avanzar a la siguiente posición
        current_x += BITMAP_SIZE + 1; // 8 píxeles + 1 de espacio
        
        // Si llegamos al borde de la pantalla, pasar a la siguiente línea
        if(current_x + BITMAP_SIZE >= VGA_WIDTH){
            current_x = x;
            y += BITMAP_SIZE + 2; // Nueva línea con espacio
        }
        
        index++;
    }
}

void fill_rect_backbuffer(short x, short y, short width, short height, char color) {
    for(int row = y; row < y + height; row++) {
        draw_line_backbuffer(x, row, x + width, row, color);
    }
}



// ========== DIBUJO WINDOWS 98 CON BACK BUFFER ==========

void draw_window_98_backbuffer(int x, int y, int width, int height, char* title, char active) {
    // Borde exterior
    draw_rect_backbuffer(x, y, width, height, WIN_DARK_GREY);
    
    // Borde interior
    draw_rect_backbuffer(x+1, y+1, width-2, height-2, WIN_LIGHT_GREY);
    
    // Barra de título
    if(active) {
        fill_rect_backbuffer(x+3, y+3, width-6, 16, WIN_BLUE);
    } else {
        fill_rect_backbuffer(x+3, y+3, width-6, 16, WIN_DARK_BLUE);
    }
    
    // Botones
    fill_rect_backbuffer(x + width - 20, y + 5, 14, 12, WIN_RED);
    draw_rect_backbuffer(x + width - 20, y + 5, 14, 12, WIN_BLACK);
    
    fill_rect_backbuffer(x + width - 40, y + 5, 14, 12, WIN_GREY);
    draw_rect_backbuffer(x + width - 40, y + 5, 14, 12, WIN_BLACK);
    draw_line_backbuffer(x + width - 37, y + 11, x + width - 27, y + 11, WIN_BLACK);
    
    fill_rect_backbuffer(x + width - 60, y + 5, 14, 12, WIN_GREY);
    draw_rect_backbuffer(x + width - 60, y + 5, 14, 12, WIN_BLACK);
    
    // Título
    int title_x = x + 8;
    int title_y = y + 6;
    int idx = 0;
    while(title[idx]) {
        draw_char(title_x, title_y, WIN_WHITE, title[idx]);
        title_x += 9;
        idx++;
    }
    
    // Área de contenido
    fill_rect_backbuffer(x+3, y+22, width-6, height-25, WIN_WHITE);
    draw_rect_backbuffer(x+3, y+22, width-6, height-25, WIN_BLACK);
}

void draw_terminal_content_backbuffer(int x, int y, int width, int height) {
    draw_string_backbuffer(x + 10, y + 30, WIN_BLACK, "C:\\WINDOWS>");
    draw_line_backbuffer(x + 10, y + 45, x + width - 20, y + 45, WIN_GREY);
    
    // Mostrar coordenadas del mouse
    draw_string_backbuffer(x + 10, y + 60, WIN_BLACK, "Mouse X: ");
    
    // Convertir coordenadas a string simple
    char x_str[4] = "   ";
    char y_str[4] = "   ";
    int temp_x = mouse_x;
    int temp_y = mouse_y;
    
    if(temp_x >= 100) {
        x_str[0] = '1';
        x_str[1] = '0';
        x_str[2] = '0';
        x_str[3] = '+';
    } else if(temp_x >= 10) {
        x_str[0] = '0' + (temp_x / 10);
        x_str[1] = '0' + (temp_x % 10);
    } else {
        x_str[0] = '0' + temp_x;
    }
    
    draw_string_backbuffer(x + 70, y + 60, WIN_BLACK, x_str);
    draw_string_backbuffer(x + 10, y + 75, WIN_BLACK, "Mouse Y: ");
    
    if(temp_y >= 100) {
        y_str[0] = '1';
        y_str[1] = '0';
        y_str[2] = '0';
        y_str[3] = '+';
    } else if(temp_y >= 10) {
        y_str[0] = '0' + (temp_y / 10);
        y_str[1] = '0' + (temp_y % 10);
    } else {
        y_str[0] = '0' + temp_y;
    }
    
    draw_string_backbuffer(x + 70, y + 75, WIN_BLACK, y_str);
    
    if(mouse_left_pressed) {
        draw_string_backbuffer(x + 10, y + 90, WIN_RED, "LEFT CLICK");
    } else {
        draw_string_backbuffer(x + 10, y + 90, WIN_GREEN, "NO CLICK");
    }
}

void draw_calculator_content_backbuffer(int x, int y, int width, int height) {
    // Pantalla
    fill_rect_backbuffer(x + 10, y + 30, width - 20, 25, WIN_BLACK);
    draw_string_backbuffer(x + 15, y + 36, WIN_GREEN, "0");
    
    // Botones
    char* buttons[4][4] = {
        {"7", "8", "9", "/"},
        {"4", "5", "6", "*"},
        {"1", "2", "3", "-"},
        {"0", ".", "=", "+"}
    };
    
    for(int row = 0; row < 4; row++) {
        for(int col = 0; col < 4; col++) {
            int btn_x = x + 15 + (col * 35);
            int btn_y = y + 70 + (row * 25);
            
            fill_rect_backbuffer(btn_x, btn_y, 30, 20, WIN_GREY);
            draw_rect_backbuffer(btn_x, btn_y, 30, 20, WIN_BLACK);
            
            // Centrar texto
            int text_x = btn_x + 10;
            if(buttons[row][col][0] == '.') text_x = btn_x + 12;
            else if(buttons[row][col][0] == '=') text_x = btn_x + 8;
            else if(buttons[row][col][0] == '+') text_x = btn_x + 12;
            else if(buttons[row][col][0] == '-') text_x = btn_x + 12;
            else if(buttons[row][col][0] == '*') text_x = btn_x + 12;
            else if(buttons[row][col][0] == '/') text_x = btn_x + 12;
            
            draw_string_backbuffer(text_x, btn_y + 5, WIN_BLACK, buttons[row][col]);
        }
    }
}

void draw_paint_content_backbuffer(int x, int y, int width, int height) {
    // Área de dibujo
    fill_rect_backbuffer(x + 10, y + 30, width - 20, height - 50, WIN_WHITE);
    draw_rect_backbuffer(x + 10, y + 30, width - 20, height - 50, WIN_BLACK);
    
    // Barra de herramientas
    fill_rect_backbuffer(x + 10, y + 35, width - 20, 20, WIN_LIGHT_GREY);
    
    // Herramientas
    fill_rect_backbuffer(x + 15, y + 38, 15, 14, WIN_BLUE);
    draw_rect_backbuffer(x + 15, y + 38, 15, 14, WIN_BLACK);
    
    fill_rect_backbuffer(x + 35, y + 38, 15, 14, WIN_RED);
    draw_rect_backbuffer(x + 35, y + 38, 15, 14, WIN_BLACK);
    draw_line_backbuffer(x + 37, y + 45, x + 48, y + 45, WIN_BLACK);
    
    fill_rect_backbuffer(x + 55, y + 38, 15, 14, WIN_GREEN);
    draw_rect_backbuffer(x + 55, y + 38, 15, 14, WIN_BLACK);
    
    fill_rect_backbuffer(x + 75, y + 38, 15, 14, WIN_YELLOW);
    draw_rect_backbuffer(x + 75, y + 38, 15, 14, WIN_BLACK);
    
    // Paleta
    for(int i = 0; i < 8; i++) {
        fill_rect_backbuffer(x + 100 + (i * 12), y + 38, 10, 10, i);
        draw_rect_backbuffer(x + 100 + (i * 12), y + 38, 10, 10, WIN_BLACK);
    }
}

void draw_taskbar_backbuffer() {
    // Barra de tareas
    fill_rect_backbuffer(0, 185, 320, 15, WIN_GREY);
    
    // Botón inicio
    fill_rect_backbuffer(2, 187, 50, 11, WIN_GREEN);
    draw_rect_backbuffer(2, 187, 50, 11, WIN_BLACK);
    draw_string_backbuffer(8, 188, WIN_BLACK, "Start");
    
    // Botones de aplicaciones
    for(int i = 0; i < 4; i++) {
        if(taskbar_buttons[i].pressed) {
            fill_rect_backbuffer(taskbar_buttons[i].x, taskbar_buttons[i].y, 70, 13, WIN_DARK_GREY);
        } else {
            fill_rect_backbuffer(taskbar_buttons[i].x, taskbar_buttons[i].y, 70, 13, WIN_LIGHT_GREY);
        }
        draw_rect_backbuffer(taskbar_buttons[i].x, taskbar_buttons[i].y, 70, 13, WIN_BLACK);
        draw_string_backbuffer(taskbar_buttons[i].x + 5, taskbar_buttons[i].y + 2, WIN_BLACK, taskbar_buttons[i].text);
        
        // Icono
        int icon_x = taskbar_buttons[i].x + 60;
        int icon_y = taskbar_buttons[i].y + 3;
        
        switch(taskbar_buttons[i].icon) {
            case 0: fill_rect_backbuffer(icon_x, icon_y, 6, 6, WIN_BLUE); break;
            case 1: fill_rect_backbuffer(icon_x, icon_y, 6, 6, WIN_GREEN); break;
            case 2: fill_rect_backbuffer(icon_x, icon_y, 6, 6, WIN_YELLOW); break;
            case 3: fill_rect_backbuffer(icon_x, icon_y, 6, 6, WIN_DARK_BLUE); break;
        }
    }
    
    // Reloj
    fill_rect_backbuffer(280, 187, 38, 11, WIN_LIGHT_GREY);
    draw_rect_backbuffer(280, 187, 38, 11, WIN_BLACK);
    draw_string_backbuffer(285, 188, WIN_BLACK, "12:00");
}

void draw_start_menu_backbuffer() {
    if(!start_menu_open) return;
    
    fill_rect_backbuffer(start_menu_x, start_menu_y - 80, 80, 80, WIN_LIGHT_GREY);
    draw_rect_backbuffer(start_menu_x, start_menu_y - 80, 80, 80, WIN_BLACK);
    
    draw_string_backbuffer(start_menu_x + 5, start_menu_y - 75, WIN_BLACK, "Programs");
    draw_line_backbuffer(start_menu_x + 5, start_menu_y - 70, start_menu_x + 75, start_menu_y - 70, WIN_DARK_GREY);
    
    draw_string_backbuffer(start_menu_x + 5, start_menu_y - 60, WIN_BLACK, "Documents");
    draw_string_backbuffer(start_menu_x + 5, start_menu_y - 50, WIN_BLACK, "Settings");
    draw_string_backbuffer(start_menu_x + 5, start_menu_y - 40, WIN_BLACK, "Find");
    draw_string_backbuffer(start_menu_x + 5, start_menu_y - 30, WIN_BLACK, "Help");
    draw_line_backbuffer(start_menu_x + 5, start_menu_y - 25, start_menu_x + 75, start_menu_y - 25, WIN_DARK_GREY);
    
    draw_string_backbuffer(start_menu_x + 5, start_menu_y - 15, WIN_BLACK, "Run...");
    draw_string_backbuffer(start_menu_x + 5, start_menu_y - 5, WIN_BLACK, "Shutdown");
}

void draw_desktop_backbuffer() {
    // Fondo azul
    fill_backbuffer(WIN_BLUE);
    
    // Iconos del escritorio
    draw_string_backbuffer(10, 30, WIN_WHITE, "My Computer");
    draw_string_backbuffer(10, 50, WIN_WHITE, "My Documents");
    draw_string_backbuffer(10, 70, WIN_WHITE, "Recycle Bin");
    draw_string_backbuffer(10, 90, WIN_WHITE, "Network");
    
    // Dibujar cuadrados como iconos
    for(int i = 0; i < 4; i++) {
        fill_rect_backbuffer(5, 25 + (i * 20), 5, 5, WIN_YELLOW);
    }
}

void draw_cursor_backbuffer() {
    int x = mouse_x;
    int y = mouse_y;
    
    // Cuerpo de la flecha
    for(int i = 0; i < 8; i++) {
        putpixel_backbuffer(x + i, y, WIN_WHITE);
    }
    for(int i = 1; i < 8; i++) {
        putpixel_backbuffer(x, y + i, WIN_WHITE);
    }
    
    // Diagonal
    for(int i = 1; i < 4; i++) {
        putpixel_backbuffer(x + i, y + i, WIN_WHITE);
    }
    
    // Puntero negro
    putpixel_backbuffer(x, y, WIN_BLACK);
    putpixel_backbuffer(x + 1, y, WIN_BLACK);
    putpixel_backbuffer(x, y + 1, WIN_BLACK);
    putpixel_backbuffer(x + 1, y + 1, WIN_BLACK);
    putpixel_backbuffer(x + 1, y + 2, WIN_BLACK);
    putpixel_backbuffer(x + 2, y + 1, WIN_BLACK);
    putpixel_backbuffer(x + 2, y + 2, WIN_BLACK);
}

// ========== FUNCIÓN PRINCIPAL ==========
int k_main(void) {
    // Inicializar VGA
    init_vga_fnc();
    
    // Inicializar mouse
    mouse_init();
    
    // Abrir terminal por defecto
    windows[0].visible = 1;
    
    // Loop principal
    while(1) {
        // Procesar mouse
        if(inb(PS2_CMD_PORT) & 1) {
            if(inb(PS2_CMD_PORT) & 0x20) {
                mouse_handler();
            }
        }
        
        // ===== DIBUJAR EN BACK BUFFER =====
        draw_desktop_backbuffer();
        draw_taskbar_backbuffer();
        draw_start_menu_backbuffer();
        
        // Dibujar ventanas
        for(int i = 0; i < 3; i++) {
            if(windows[i].visible) {
                // Determinar si está activa (si se está arrastrando o es la última abierta)
                char active = windows[i].dragging;
                for(int j = i + 1; j < 3; j++) {
                    if(windows[j].visible && windows[j].dragging) {
                        active = 0;
                        break;
                    }
                }
                
                draw_window_98_backbuffer(windows[i].x, windows[i].y,
                                         windows[i].width, windows[i].height,
                                         windows[i].title, active);
                
                // Dibujar contenido
                switch(windows[i].content_id) {
                    case 0:
                        draw_terminal_content_backbuffer(windows[i].x, windows[i].y,
                                                        windows[i].width, windows[i].height);
                        break;
                    case 1:
                        draw_calculator_content_backbuffer(windows[i].x, windows[i].y,
                                                          windows[i].width, windows[i].height);
                        break;
                    case 2:
                        draw_paint_content_backbuffer(windows[i].x, windows[i].y,
                                                     windows[i].width, windows[i].height);
                        break;
                }
            }
        }
        
        // Dibujar cursor (siempre al final)
        draw_cursor_backbuffer();
        
        // ===== CAMBIAR BUFFERS (SIN PARPADEO) =====
        swap_buffers();
        
        // Pequeño delay
        delay(3000);
    }
    
    return 0;
}