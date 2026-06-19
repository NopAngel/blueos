#include <drivers/vt100.h>
#include <kernel/colors.h>
#include <kernel/printk.h>
#include <kernel/timer.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#define LOGLEVEL_EMERG 0 /* System is unusable */                // KERN_EMERG
#define LOGLEVEL_ALERT 1 /* Action must be taken immediately */  // KERN_ALERT
#define LOGLEVEL_CRIT 2 /* Critical conditions */                // KERN_CRIT
#define LOGLEVEL_ERR 3 /* Error conditions */                    // KERN_ERR
#define LOGLEVEL_WARNING 4 /* Warning conditions */              // KERN_WARNING
#define LOGLEVEL_NOTICE 5 /* Normal but significant condition */ // KERN_NOTICE
#define LOGLEVEL_INFO 6 /* Informational */                      // KERN_INFO
#define LOGLEVEL_DEBUG 7 /* Debug-level messages */              // KERN_DEBUG
#define LOGLEVEL_DEFAULT LOGLEVEL_WARNING // Default log level if not specified

/* --- Global Variables --- */
int console_loglevel =
    LOGLEVEL_DEBUG; // Maximum log level to display on the console
int default_message_loglevel =
    LOGLEVEL_DEFAULT; // Default log level if not specified
char kernel_log_buffer[LOG_BUFFER_SIZE];
uint32_t log_ptr = 0;
static int at_line_start = 1;

int g_gui_enabled = 0;

extern void update_cursor();

/* External cursors (defined in your arch-specific task or boot files) */
// These externs are architecture-specific and assumed to be defined elsewhere.
extern int cursor_x;
extern int cursor_y;
extern uint64_t timer_get_ms(void); // Required for print_timestamp

/* --- Hardware Definitions --- */

#if defined(__riscv) || defined(RISCV)
/* RISC-V QEMU Virt Machine UART0 */
#define UART0_BASE 0x10000000
#define UART_DR ((volatile uint8_t *)(UART0_BASE + 0))
#define UART_LSR ((volatile uint8_t *)(UART0_BASE + 5))
#define UART_LSR_EMPTY 0x20

#elif defined(__x86__) || defined(x86)
/* x86 VGA Video Memory */
#define VIDEO_MEM ((char *)0xb8000)
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

#endif

/* Forward declarations */
static void __putchar(char c);
static void log_char(char c);
static void log_int(long num, int base, int precision);
static void log_timestamp();
void print_int(long num, int base, int precision);

/* --- Low Level Output --- */

/**
 * log_char: Writes a character only to the circular log buffer (dmesg)
 */
static void log_char(char c) {
  kernel_log_buffer[log_ptr] = c;
  log_ptr = (log_ptr + 1) % LOG_BUFFER_SIZE;
}

/**
 * log_int: Helper to write integers only to the log buffer for the timestamp
 */
static void log_int(long num, int base, int precision) {
  char buffer[32];
  int i = 0;
  unsigned long n = (num < 0 && base == 10) ? -num : (unsigned long)num;
  if (num < 0 && base == 10)
    log_char('-');
  char hex_chars[] = "0123456789abcdef";
  do {
    buffer[i++] = hex_chars[n % base];
    n /= base;
  } while (n > 0);
  while (i < precision)
    buffer[i++] = '0';
  while (i > 0)
    log_char(buffer[--i]);
}

/**
 * log_timestamp: Records the system timestamp ONLY in the circular buffer (for
 * dmesg)
 */
static void log_timestamp() {
  uint64_t ms = timer_get_ms();
  log_char('[');
  log_int(ms / 1000, 10, 5);
  log_char('.');
  log_int(ms % 1000, 10, 3);
  log_char(']');
  log_char(' ');
}

/**
 * putchar: Now uses the VT100 engine instead of writing directly to hardware.
 */
static void __putchar(char c) {
  // If it's the start of a line, record the timestamp ONLY in the internal log
  // (dmesg)
  if (at_line_start && c != '\n' && c != '\r' && c != '\033') {
    log_timestamp();
    at_line_start = 0;
  }

  // Visual output using the VT100 driver
  vt100_putc(c);

  // The message is always stored in the circular buffer
  log_char(c);

  // Update line state tracking
  if (c == '\n')
    at_line_start = 1;
  else if (c != '\r')
    at_line_start = 0;
}

/**
 * print_int: Helper to convert integers to strings and print them.
 */
void print_int(long num, int base, int precision) {
  char buffer[32];
  int i = 0;
  unsigned long n = (num < 0 && base == 10) ? -num : (unsigned long)num;

  if (num < 0 && base == 10) {
    __putchar('-');
  }

  char hex_chars[] = "0123456789abcdef";
  do {
    buffer[i++] = hex_chars[n % base];
    n /= base;
  } while (n > 0);

  /* Fill leading zeros if precision is requested */
  while (i < precision)
    buffer[i++] = '0';

  /* Print buffer in reverse */
  while (i > 0) {
    char c = buffer[--i];
    __putchar(c);
  }
}

/* --- High Level Printk Implementation --- */

unsigned int vprintk(const char *fmt, va_list args) {
  for (const char *p = fmt; *p != '\0'; p++) {
    if (*p != '%') {
      __putchar(*p);
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
      if (!s)
        s = "(null)";
      while (*s) {
        __putchar(*s++);
      }
      break;
    }
    case 'd':
    case 'i':
      print_int(va_arg(args, int), 10, precision);
      break;
    case 'u':
      print_int(va_arg(args, unsigned int), 10, precision);
      break;
    case 'x':
    case 'X':
      print_int(va_arg(args, unsigned int), 16, precision);
      break;
    case 'p':
      __putchar('0');
      __putchar('x');
      print_int(va_arg(args, unsigned long), 16, 8);
      break;
    case 'c': {
      char c = (char)va_arg(args, int);
      __putchar(c);
      break;
    }
    case '%':
      __putchar('%');
      break;
    }
  }
  return 1;
}

/**
 * printk: Main kernel logging function with color and loglevel support.
 */
unsigned int printk(const char *fmt, ...) {
  va_list args;
  const char *p = fmt;
  int msg_level = default_message_loglevel;

  /* Check for loglevel prefix: <n> */
  if (fmt[0] == '<' && fmt[1] >= '0' && fmt[1] <= '7' && fmt[2] == '>') {
    msg_level = fmt[1] - '0';
    p = fmt + 3;
  }

  /* Filter by console_loglevel */
  if (msg_level > console_loglevel)
    return 0;

  va_start(args, fmt);
  unsigned int result = vprintk(p, args);
  va_end(args);

  return result;
}

void clear_screen() {
#if defined(__riscv) || defined(RISCV)
  /* ANSI escape code to clear terminal and move cursor to home */
  printk("\033[2J\033[H");
#elif defined(__x86__) || defined(x86)
  for (int i = 0; i < (VGA_WIDTH * VGA_HEIGHT * 2); i += 2) {
    VIDEO_MEM[i] = ' ';
    VIDEO_MEM[i + 1] = 0x07;
  }
#endif
  cursor_x = 0;
  cursor_y = 0;
#if defined(x86)
  update_cursor();
#endif
}

/**
 * sprintf_int: Internal helper for sprintf to convert integers to strings.
 */
static char *sprintf_int(char *buf, long num, int base, int precision) {
  char temp[32];
  int i = 0;
  unsigned long n = (num < 0 && base == 10) ? -num : (unsigned long)num;

  if (num < 0 && base == 10) {
    *buf++ = '-';
  }

  char hex_chars[] = "0123456789abcdef";
  do {
    temp[i++] = hex_chars[n % base];
    n /= base;
  } while (n > 0);

  /* Fill leading zeros if precision is requested */
  while (i < precision)
    temp[i++] = '0';

  /* Copy to buffer in reverse */
  while (i > 0) {
    *buf++ = temp[--i];
  }
  return buf;
}

/**
 * vsprintf: Formats a string into a buffer using a va_list.
 */
int vsprintf(char *buf, const char *fmt, va_list args) {
  char *p_buf = buf;
  for (const char *p = fmt; *p != '\0'; p++) {
    if (*p != '%') {
      *p_buf++ = *p;
      continue;
    }

    p++; /* Move past '%' */

    /* Parser de precisión mejorado para manejar más de un dígito */
    int precision = 0;
    if (*p == '0') {
      p++;
      while (*p >= '0' && *p <= '9') {
        precision = precision * 10 + (*p - '0');
        p++;
      }
    }

    switch (*p) {
    case 's': {
      char *s = va_arg(args, char *);
      if (!s)
        s = "(null)";
      while (*s)
        *p_buf++ = *s++;
      break;
    }
    case 'd':
    case 'i':
      p_buf = sprintf_int(p_buf, va_arg(args, int), 10, precision);
      break;
    case 'u':
      p_buf = sprintf_int(p_buf, va_arg(args, unsigned int), 10, precision);
      break;
    case 'x':
    case 'X':
      p_buf = sprintf_int(p_buf, va_arg(args, unsigned int), 16, precision);
      break;
    case 'p':
      *p_buf++ = '0';
      *p_buf++ = 'x';
      p_buf = sprintf_int(p_buf, va_arg(args, unsigned long), 16, 8);
      break;
    case 'c':
      *p_buf++ = (char)va_arg(args, int);
      break;
    case '%':
      *p_buf++ = '%';
      break;
    }
  }
  *p_buf = '\0';
  return (int)(p_buf - buf);
}

/**
 * sprintf: Formats a string into a buffer.
 */
int sprintf(char *buf, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  int result = vsprintf(buf, fmt, args);
  va_end(args);
  return result;
}

/**
 * boot_msg - Professional styled boot logging
 * Status: 0 = OK, 1 = WARN, 2 = FAIL
 */
void boot_msg(const char *subsystem, const char *msg, int status) {
  switch (status) {
  case 0:
    printk("[  \033[32mOK\033[0m  ] %s: %s\n", subsystem, msg);
    break;
  case 1:
    printk("[  \033[33mWARN\033[0m  ] %s: %s\n", subsystem, msg);
    break;
  case 2:
    printk("[  \033[31mFAIL\033[0m  ] %s: %s\n", subsystem, msg);
    break;
  default:
    printk("[  INFO  ] %s: %s\n", subsystem, msg);
    break;
  }
  vt100_set_color(0x0F);
}