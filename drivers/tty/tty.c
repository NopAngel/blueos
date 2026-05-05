#include <drivers/keyboard.h>
#include <kernel/printk.h>
#include <kernel/colors.h>
#include <lib/string.h>

#define TTY_BUFFER_SIZE 1024
#define CTRL(c) ((c) & 0x1F)

/* --- Estructura termios (Estilo Linux) --- */
typedef struct {
    uint32_t c_iflag;      /* input modes */
    uint32_t c_oflag;      /* output modes */
    uint32_t c_lflag;      /* local modes (ECHO, ICANON, etc) */
    uint8_t  c_cc[20];     /* control characters (VINTR, VEOF...) */
} struct_termios;

/* Flags básicos de Linux */
#define ICANON  0000002    /* Modo canónico (por líneas) */
#define ECHO    0000010    /* Hacer eco de los caracteres */
#define ISIG    0000001    /* Habilitar señales (Ctrl+C) */

typedef struct {
    char buffer[TTY_BUFFER_SIZE];
    uint32_t head;
    uint32_t tail;
    uint32_t count;
    struct_termios conf;
    void (*write_out)(char c); /* Función para escupir al hardware */
} tty_t;

static tty_t main_tty;

/**
 * tty_init: Inicializa el TTY estilo Linux
 */
void tty_init(void (*hw_write)(char)) {
    memset(&main_tty, 0, sizeof(tty_t));
    main_tty.write_out = hw_write;

    /* Configuración por defecto: Modo Linux clásico */
    main_tty.conf.c_lflag = ICANON | ECHO | ISIG;
    main_tty.conf.c_cc[0] = CTRL('c'); /* VINTR */

    printk(CYAN, "TTY: BlueOS driver initialized\n");
}

/**
 * tty_input: El "Line Discipline" (N_TTY)
 * Aquí es donde el driver de teclado o UART inyecta datos
 */
void tty_input(char c) {
    /* 1. Manejo de señales (ISIG) */
    if (main_tty.conf.c_lflag & ISIG) {
        if (c == main_tty.conf.c_cc[0]) {
            printk(RED, "\n^C - SIGINT recibido (BlueOS Signal)\n");
            // signal_send(current_process, SIGINT);
            return;
        }
    }

    /* 2. Modo Canónico (ICANON) */
    if (main_tty.conf.c_lflag & ICANON) {
        if (c == '\b' || c == 127) {
            if (main_tty.count > 0) {
                main_tty.head = (main_tty.head - 1) % TTY_BUFFER_SIZE;
                main_tty.count--;
                if (main_tty.conf.c_lflag & ECHO) main_tty.write_out('\b');
            }
            return;
        }
    }

    /* 3. Almacenar en el buffer circular */
    if (main_tty.count < TTY_BUFFER_SIZE) {
        main_tty.buffer[main_tty.head] = c;
        main_tty.head = (main_tty.head + 1) % TTY_BUFFER_SIZE;
        main_tty.count++;

        /* 4. ECO */
        if (main_tty.conf.c_lflag & ECHO) {
            main_tty.write_out(c);
        }
    }
}

/**
 * tty_read: Lo que llamaría la syscall 'read' desde el espacio de usuario
 */
int tty_read(char* user_buf, int n) {
    int i = 0;
    while (i < n && main_tty.count > 0) {
        user_buf[i++] = main_tty.buffer[main_tty.tail];
        main_tty.tail = (main_tty.tail + 1) % TTY_BUFFER_SIZE;
        main_tty.count--;
    }
    return i;
}
