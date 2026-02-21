#include <include/printk.h>
#include <include/colors.h>
#include <stdarg.h>
#include <stddef.h>

int redirect_to_file = 0;
char* redirect_buffer = NULL;
int redirect_ptr = 0;

extern int cursor_x;
extern int cursor_y;
extern void put_char(char c, unsigned int color);
void putchar(char c, unsigned int color) {
    char *vidmem = (char *) 0xb8000;
    
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else {
        int index = (cursor_y * 80 + cursor_x) * 2;
        vidmem[index] = c;
        vidmem[index + 1] = (char)color;
        cursor_x++;
    }

    if (cursor_x >= 80) {
        cursor_x = 0;
        cursor_y++;
    }
}

void print_int(int num, int base, unsigned int color) {
    char buffer[32];
    int i = 0;
    unsigned int n = (num < 0 && base == 10) ? -num : num;
    if (num < 0 && base == 10) putchar('-', color);
    do {
        int rem = n % base;
        buffer[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        n /= base;
    } while (n > 0);
    while (i > 0) putchar(buffer[--i], color);
}

unsigned int printk(unsigned int color, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    if (redirect_to_file && redirect_buffer != NULL) {
        return;
    }
    for (const char *p = fmt; *p != '\0'; p++) {
        if (*p != '%') {
            put_char(*p, color);
            continue;
        }

        p++; 
        
        switch (*p) {
            case 's': {
                char *s = va_arg(args, char *);
                if (!s) s = "(null)";
                while (*s) {
                    put_char(*s++, color);
                }
                break;
            }
            case 'd': {
                int d = va_arg(args, int);
                print_int(d, 10, color);
                break;
            }
            case 'x': {
                put_char('0', color);
                put_char('x', color);
                int x = va_arg(args, int);
                print_int(x, 16, color);
                break;
            }
            case 'c': {
                char c = (char)va_arg(args, int);
                put_char(c, color);
                break;
            }
            case '%': {
                put_char('%', color);
                break;
            }
            default:
                put_char('%', color);
                put_char(*p, color);
                break;
        }
    }

    va_end(args);
    return 1;
}

void clear_screen() {
    char *vidmem = (char *) 0xb8000;
    for (int i = 0; i < (80 * 25 * 2); i += 2) {
        vidmem[i] = ' ';
        vidmem[i+1] = WHITE;
    }
    cursor_x = 0;
    cursor_y = 0;
}

