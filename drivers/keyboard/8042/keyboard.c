#include <config.h>
#include <drivers/keyboard.h>
#include <kernel/printk.h>
#include <kernel/colors.h>
#include <kernel/commands.h>
#include <lib/string.h>

#define KEYBOARD_BUFFER_SIZE 256
static int head = 0;
static int tail = 0;
static char keyboard_buffer[KEYBOARD_BUFFER_SIZE];
/* --- TTY-Style State --- */
typedef struct {
    char buffer[INPUT_BUFFER_SIZE];
    int index;
    int echo;           /* Si es 0, no imprime lo que escribes (modo pass) */
    int raw_mode;       /* Si es 1, manda cada tecla sin esperar a Enter */
    int enabled;
} tty_state_t;

static tty_state_t kbd_tty = { .index = 0, .echo = 1, .raw_mode = 0, .enabled = 1 };

/* Scancode to ASCII Tables */
#if defined(I386)
static const char kbd_map[] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};
static int shift_state = 0;
#endif

/* --- API de Control (Estilo Linux termios) --- */
void kbd_set_echo(int enable) { kbd_tty.echo = enable; }

/**
 * handle_erase: Maneja el retroceso visual y lógico
 */
static void handle_erase() {
    if (kbd_tty.index > 0) {
        kbd_tty.index--;
        arch_put_char('\b', WHITE);
    }
}

/**
 * process_character: La "Disciplina de Terminal"
 */
static void process_character(char c) {
    if (c == 0) return;

    /* 1. Manejo de teclas especiales */
    switch (c) {
        case '\n':
        case '\r':
            arch_put_char('\n', WHITE);
            kbd_tty.buffer[kbd_tty.index] = '\0';

            if (kbd_tty.index > 0) {
                execute_shell_command(kbd_tty.buffer);
            } else {
                print_prompt();
            }
            kbd_tty.index = 0;
            break;

        case '\b':
        case 127: /* DEL en algunos sistemas */
            handle_erase();
            break;

        case '\t':
            /* Autocompletado (Podrías llamar a tu lógica de tab aquí) */
            break;

        default:
            /* 2. Caracteres imprimibles */
            if (c >= 32 && c <= 126 && kbd_tty.index < INPUT_BUFFER_SIZE - 1) {
                kbd_tty.buffer[kbd_tty.index++] = c;
                if (kbd_tty.echo) {
                    arch_put_char(c, WHITE);
                }
            }
            break;
    }
}

char get_char() {
    // (Busy wait)
    while (head == tail) {
        asm volatile("hlt");
    }

    char c = keyboard_buffer[tail];
    tail = (tail + 1) % KEYBOARD_BUFFER_SIZE;
    return c;
}

/**
 * keyboard_handler: Punto de entrada IRQ/Polling
 */
void keyboard_handler() {
    if (!kbd_tty.enabled) return;

    char c = 0;

#if defined(RISCV)
    uint8_t raw = arch_get_scancode();
    if (raw == 0) return;
    c = (char)raw;
    /* Normalización estilo Linux */
    if (c == '\r') c = '\n';
    if (c == 127)  c = '\b';

#elif defined(I386)
    uint8_t scancode = arch_get_scancode();
    if (scancode == 0) return;

    /* Manejo de liberación de tecla (Bit 7) */
    if (scancode & 0x80) {
        uint8_t released = scancode & 0x7F;
        if (released == 0x2A || released == 0x36) shift_state = 0;
        return;
    }

    /* Modificadores */
    if (scancode == 0x2A || scancode == 0x36) { shift_state = 1; return; }

    /* Mapeo */
    if (scancode < sizeof(kbd_map)) {
        c = kbd_map[scancode];
        if (shift_state && c >= 'a' && c <= 'z') c -= 32;
    }
#endif

    process_character(c);
}
