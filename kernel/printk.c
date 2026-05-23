#include <kernel/printk.h>
#include <kernel/colors.h>
#include <stdarg.h>
#include <stddef.h>



/* --- Global Variables --- */
int console_loglevel = 7;
char kernel_log_buffer[LOG_BUFFER_SIZE];
uint32_t log_ptr = 0;

extern void update_cursor();

/* External cursors (defined in your arch-specific task or boot files) */
extern int cursor_x;
extern int cursor_y;
extern void arch_put_char(char c, unsigned int color);

/* --- Hardware Definitions --- */

#if defined(__riscv) || defined(RISCV)
    /* RISC-V QEMU Virt Machine UART0 */
    #define UART0_BASE 0x10000000
    #define UART_DR    ((volatile uint8_t *)(UART0_BASE + 0))
    #define UART_LSR   ((volatile uint8_t *)(UART0_BASE + 5))
    #define UART_LSR_EMPTY 0x20

#elif defined(__x86__) || defined(x86)
    /* x86 VGA Video Memory */
    #define VIDEO_MEM  ((char *)0xb8000)
    #define VGA_WIDTH  80
    #define VGA_HEIGHT 25

#endif

/* --- Low Level Output --- */

/**
 * putchar: Outputs a single character to the hardware.
 * Detects architecture at compile-time to use UART or VGA memory.
 */
void
putchar(char c, unsigned int color)
{
	arch_put_char(c, color);
}

/**
 * add_to_kernel_log_char: Internal circular buffer for dmesg-like logging.
 */
void
add_to_kernel_log_char(char c)
{
    kernel_log_buffer[log_ptr] = c;
    log_ptr = (log_ptr + 1) % LOG_BUFFER_SIZE;
}

/**
 * print_int: Helper to convert integers to strings and print them.
 */
void
print_int(long num, int base, unsigned int color, int precision)
{
    char buffer[32];
    int i = 0;
    unsigned long n = (num < 0 && base == 10) ? -num : (unsigned long)num;

    if (num < 0 && base == 10) {
        putchar('-', color);
        add_to_kernel_log_char('-');
    }

    char hex_chars[] = "0123456789abcdef";
    do {
        buffer[i++] = hex_chars[n % base];
        n /= base;
    } while (n > 0);

    /* Fill leading zeros if precision is requested */
    while (i < precision) buffer[i++] = '0';

    /* Print buffer in reverse */
    while (i > 0) {
        char c = buffer[--i];
        putchar(c, color);
        add_to_kernel_log_char(c);
    }
}

/* --- High Level Printk Implementation --- */

unsigned int
vprintk(unsigned int color, const char *fmt, va_list args)
{
    for (const char *p = fmt; *p != '\0'; p++) {
        if (*p != '%') {
            putchar(*p, color);
            add_to_kernel_log_char(*p);
            continue;
        }

        p++; /* Move past '%' */

        /* Check for padding/precision (e.g., %08x) */
        int precision = 0;
        if (*p == '0') {
            p++;
            if (*p >= '0' && *p <= '9') {
                precision = *p - '0';
                p++;
            }
        }

        switch (*p) {
            case 's': {
                char *s = va_arg(args, char *);
                if (!s) s = "(null)";
                while (*s) {
                    putchar(*s, color);
                    add_to_kernel_log_char(*s++);
                }
                break;
            }
            case 'd':
            case 'i':
                print_int(va_arg(args, int), 10, color, precision);
                break;
            case 'u':
                print_int(va_arg(args, unsigned int), 10, color, precision);
                break;
            case 'x':
            case 'X':
                print_int(va_arg(args, unsigned int), 16, color, precision);
                break;
        case 'p':
                putchar('0', color);
                putchar('x', color);
                print_int(va_arg(args, unsigned long), 16, color, 8);
                break;
            case 'c': {
                char c = (char)va_arg(args, int);
                putchar(c, color);
                add_to_kernel_log_char(c);
                break;
            }
            case '%':
                putchar('%', color);
                add_to_kernel_log_char('%');
                break;
        }
    }
    return 1;
}

/**
 * printk: Main kernel logging function with color and loglevel support.
 */
unsigned int
printk(unsigned int color, const char *fmt, ...)
{
    va_list args;
    const char *p = fmt;
    int msg_level = 4; /* Default to Warning level */

    /* Check for loglevel prefix: <n> */
    if (fmt[0] == '<' && fmt[1] >= '0' && fmt[1] <= '7' && fmt[2] == '>') {
        msg_level = fmt[1] - '0';
        p = fmt + 3;
    }

    /* Filter by console_loglevel */
    if (msg_level > console_loglevel) return 0;

    va_start(args, fmt);
    unsigned int result = vprintk(color, p, args);
    va_end(args);

    return result;
}

void
clear_screen()
{
#if defined(__riscv) || defined(RISCV)
    /* ANSI escape code to clear terminal and move cursor to home */
    printk(WHITE, "\033[2J\033[H");
#elif defined(__x86__) || defined(x86)
    for (int i = 0; i < (VGA_WIDTH * VGA_HEIGHT * 2); i += 2) {
        VIDEO_MEM[i] = ' ';
        VIDEO_MEM[i+1] = 0x07;
    }
#endif
    cursor_x = 0;
    cursor_y = 0;
#if defined(x86)
    update_cursor();
#endif
}
