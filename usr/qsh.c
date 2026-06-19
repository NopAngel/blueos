#include <drivers/keyboard.h>
#include <fs/fs.h>
#include <fs/vfs.h>
#include <kernel/commands.h>
#include <kernel/hal.h>
#include <kernel/io.h>
#include <kernel/printk.h>
#include <kernel/timer.h>
#include <lib/string.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef INPUT_BUFFER_SIZE
#define INPUT_BUFFER_SIZE 256
#endif

#define qsh_HISTORY_SIZE 32
#define qsh_LINE_MAX 256

static char qsh_history[qsh_HISTORY_SIZE][qsh_LINE_MAX];
static int qsh_history_count = 0;
static int qsh_history_index = -1;

/* Global scancode helper for arrow keys */
extern uint8_t arch_get_scancode();

typedef enum {
  SFILE,   /* Reading from a file */
  SSTRING, /* Reading from a string (eval/cmdline) */
  SSTDIN   /* Interactive mode */
} SourceType;

typedef struct Source {
  SourceType type;
  char *str;
  int line;
  const char *file;
  vfs_node_t *node;
  struct Source *next;
} Source;

typedef struct env {
  struct env *oenv; /* Outer environment */
  Source *source;   /* Current input source */
  int type;         /* Environment type (E_PARSE, E_FUNC, etc) */
} qsh_env_t;

/* qsh Global State */
static qsh_env_t *e;
static Source *source_stack = NULL;
static int exstat = 0; /* Exit status of last command */

extern void expand_variables(char *input, char *output);
extern int execute_single_with_return(char *input);
extern char raw_get_char();
extern FileEntry file_table[];
extern unsigned int total_memory_kb;
extern uint64_t used_memory_kb;

/* ANSI Color Helpers */
#define qsh_CLR_PROMPT "\033[1;36m" /* Bold Cyan */
#define qsh_CLR_ARROW "\033[1;33m"  /* Bold Yellow */
#define qsh_CLR_RESET "\033[0m"
#define qsh_CLR_INFO "\033[1;32m" /* Bold Green */
#define qsh_CLR_ERR "\033[1;31m"  /* Bold Red */

/* --- Built-in Command Implementations --- */

static int qsh_builtin_uptime() {
  extern uint32_t system_ticks;
  uint32_t ms = system_ticks * 10; // Suponiendo ticks de 10ms
  printk(qsh_CLR_INFO "Uptime: " qsh_CLR_RESET "%d ms (%d seconds)\n", ms,
         ms / 1000);
  return 0;
}

static int qsh_builtin_free() {
  uint32_t total = total_memory_kb / 1024;
  uint32_t used = (uint32_t)(used_memory_kb / 1024);
  printk(qsh_CLR_INFO "Memory: " qsh_CLR_RESET
                      "Total: %d MB, Used: %d MB, Free: %d MB\n",
         total, used, total - used);
  return 0;
}

static int qsh_builtin_cpuinfo() {
  printk(qsh_CLR_INFO "CPU Info:\n" qsh_CLR_RESET);
#ifdef x86
  printk("  Arch: x86 (Intel/AMD Compatible)\n");
  printk("  Mode: Protected Mode (32-bit Kernel)\n");
#else
  printk("  Arch: RISC-V\n");
#endif
  return 0;
}

/* --- qsh Engine Core --- */

void qsh_exec_line(char *line) {
  if (!line || strlen(line) == 0)
    return;

  // 1. Guardar en el historial
  if (qsh_history_count == 0 ||
      strcmp(qsh_history[(qsh_history_count - 1) % qsh_HISTORY_SIZE], line) !=
          0) {
    int idx = qsh_history_count % qsh_HISTORY_SIZE;
    strncpy(qsh_history[idx], line, qsh_LINE_MAX - 1);
    qsh_history_count++;
  }
  qsh_history_index = qsh_history_count;

  // 2. Manejar Built-ins locales primero
  if (strcmp(line, "uptime") == 0) {
    qsh_builtin_uptime();
    return;
  }
  if (strcmp(line, "free") == 0) {
    qsh_builtin_free();
    return;
  }
  if (strcmp(line, "cpuinfo") == 0) {
    qsh_builtin_cpuinfo();
    return;
  }
  if (strcmp(line, "history") == 0) {
    for (int i = 0;
         i < (qsh_history_count > qsh_HISTORY_SIZE ? qsh_HISTORY_SIZE
                                                   : qsh_history_count);
         i++)
      printk("%d: %s\n", i, qsh_history[i]);
    return;
  }

  // 3. Manejar Pipes básicos (cmd1 | cmd2)
  char *pipe_ptr = strchr(line, '|');
  if (pipe_ptr) {
    *pipe_ptr = '\0';
    char *cmd2 = pipe_ptr + 1;
    while (*cmd2 == ' ')
      cmd2++;

    printk(qsh_CLR_ARROW "[Pipe] " qsh_CLR_RESET "Executing: %s then %s\n",
           line, cmd2);
    execute_single_with_return(line);
    exstat = execute_single_with_return(cmd2);
    return;
  }

  // 4. Manejar Operadores Lógicos (&&)
  char *and_ptr = strstr(line, "&&");
  if (and_ptr) {
    *and_ptr = '\0';
    char *cmd2 = and_ptr + 2;
    while (*cmd2 == ' ')
      cmd2++;

    int res = execute_single_with_return(line);
    if (res == 0)
      exstat = execute_single_with_return(cmd2);
    return;
  }

  // 5. Asignación de variables locales (VAR=VAL)
  char *eq = strchr(line, '=');
  if (eq && line[0] != ' ') {
    *eq = '\0';
    extern void set_env_var(char *name, char *value);
    set_env_var(line, eq + 1);
    return;
  }

  // 6. Ejecución estándar
  exstat = execute_single_with_return(line);
}

static char qsh_scancode_to_ascii(uint8_t sc) {
  // Mapa simplificado basado en el set 1 de teclado US
  static const char kbd_map[128] = {
      0,   27,  '1',  '2',  '3',  '4', '5', '6',  '7', '8', '9', '0',
      '-', '=', '\b', '\t', 'q',  'w', 'e', 'r',  't', 'y', 'u', 'i',
      'o', 'p', '[',  ']',  '\n', 0,   'a', 's',  'd', 'f', 'g', 'h',
      'j', 'k', 'l',  ';',  '\'', '`', 0,   '\\', 'z', 'x', 'c', 'v',
      'b', 'n', 'm',  ',',  '.',  '/', 0,   '*',  0,   ' '};
  if (sc < 128)
    return kbd_map[sc];
  return 0;
}

void qsh_print_prompt() {
  // Mostrar el usuario actual si está disponible
  extern char current_user[];
  if (current_user[0] != '\0')
    printk(qsh_CLR_PROMPT "%s@blueos", current_user);
  else
    printk(qsh_CLR_PROMPT "blueos-qsh");

  printk(qsh_CLR_PROMPT "blueos-qsh" qsh_CLR_ARROW " # " qsh_CLR_RESET);
}

/* Helper para evaluar condiciones de qsh [ val1 == val2 ] */
int qsh_eval_condition(char *condition) {
  char buf[128];
  strncpy(buf, condition, 127);

  char *start = strchr(buf, '[');
  char *end = strrchr(buf, ']');
  if (!start || !end)
    return 0;

  *end = '\0';
  char *content = start + 1;
  while (*content == ' ')
    content++;

  char val1[64], op[5], val2[64];
  char *space1 = strchr(content, ' ');
  if (!space1)
    return 0;
  *space1 = '\0';
  strcpy(val1, content);

  char *content2 = space1 + 1;
  while (*content2 == ' ')
    content2++;
  char *space2 = strchr(content2, ' ');
  if (!space2)
    return 0;
  *space2 = '\0';
  strcpy(op, content2);

  char *content3 = space2 + 1;
  while (*content3 == ' ')
    content3++;
  strcpy(val2, content3);

  if (strcmp(op, "==") == 0 || strcmp(op, "=") == 0)
    return strcmp(val1, val2) == 0;
  if (strcmp(op, "!=") == 0)
    return strcmp(val1, val2) != 0;

  return 0;
}

int qsh_shell(Source *s) {
  char line[INPUT_BUFFER_SIZE];
  int ptr = 0;
  int cursor = 0;
  char *content = NULL;
  int content_pos = 0;
  bool skip_logic = false;
  bool condition_met = false;

  if (s->type == SFILE) {
    content = (char *)s->node->ptr;
    ;
  }

  while (1) {
    ptr = 0;
    if (s->type == SFILE) {
      if (content[content_pos] == '\0')
        break;
      while (content[content_pos] != '\0' && content[content_pos] != '\n' &&
             content[content_pos] != '\r') {
        if (ptr < INPUT_BUFFER_SIZE - 1)
          line[ptr++] = content[content_pos];
        content_pos++;
      }
      if (content[content_pos] == '\r')
        content_pos++;
      if (content[content_pos] == '\n')
        content_pos++;
      line[ptr] = '\0';
    } else if (s->type == SSTDIN) {
      qsh_print_prompt();
      ptr = 0;
      cursor = 0;
      memset(line, 0, INPUT_BUFFER_SIZE);

      while (ptr < INPUT_BUFFER_SIZE - 1) {
        uint8_t sc = arch_get_scancode();
        if (sc == 0)
          continue;

        if (sc == 0x1C) { // ENTER
          printk("\n");
          line[ptr] = '\0';
          break;
        } else if (sc == 0x0E) { // BACKSPACE
          if (ptr > 0 && cursor > 0) {
            for (int i = cursor - 1; i < ptr - 1; i++)
              line[i] = line[i + 1];
            ptr--;
            cursor--;
            line[ptr] = '\0';
            printk(
                "\b \b"); // Simplificado: el kernel debería redibujar la línea
          }
        } else if (sc == 0x48) { // UP ARROW (History)
          if (qsh_history_count > 0) {
            if (qsh_history_index > 0)
              qsh_history_index--;
            strcpy(line, qsh_history[qsh_history_index % qsh_HISTORY_SIZE]);
            ptr = cursor = strlen(line);
            printk("\r");
            qsh_print_prompt();
            printk("%s   ", line);
            for (int i = 0; i < 3; i++)
              printk("\b");
          }
        } else if (sc == 0x50) { // DOWN ARROW
          if (qsh_history_index < qsh_history_count) {
            qsh_history_index++;
            if (qsh_history_index < qsh_history_count)
              strcpy(line, qsh_history[qsh_history_index % qsh_HISTORY_SIZE]);
            else {
              line[0] = '\0';
            }
            ptr = cursor = strlen(line);
            printk("\r");
            qsh_print_prompt();
            printk("%s   ", line);
            for (int i = 0; i < 3; i++)
              printk("\b");
          }
        } else if (sc == 0x4B) { // LEFT ARROW
          if (cursor > 0) {
            cursor--;
            printk("\b");
          }
        } else if (sc == 0x4D) { // RIGHT ARROW
          if (cursor < ptr) {
            char b[2] = {line[cursor], '\0'};
            printk(b);
            cursor++;
          }
        } else {
          char c = qsh_scancode_to_ascii(sc);
          if (c >= 32 && c <= 126) {
            // Insertar en la posición del cursor
            for (int i = ptr; i > cursor; i--)
              line[i] = line[i - 1];
            line[cursor] = c;
            ptr++;

            // Mostrar carácter y el resto de la línea
            for (int i = cursor; i < ptr; i++) {
              char b[2] = {line[i], '\0'};
              printk(b);
            }
            cursor++;
            // Volver el cursor visual a su sitio
            for (int i = ptr; i > cursor; i--)
              printk("\b");
          }
        }
        // Pequeña espera para no saturar el polling del teclado
        for (volatile int d = 0; d < 100000; d++)
          ;
      }

      if (strcmp(line, "exit") == 0)
        return 0;
      if (strcmp(line, "clear") == 0) {
        clear_screen();
        continue;
      }

      qsh_exec_line(line);
      continue;
    } else
      break;

    s->line++;
    char *cmd_line = line;
    while (*cmd_line == ' ' || *cmd_line == '\t')
      cmd_line++;

    if (strlen(cmd_line) > 0 && cmd_line[0] != '#') {
      char expanded[INPUT_BUFFER_SIZE];
      expand_variables(cmd_line, expanded);

      if (strncmp(expanded, "if ", 3) == 0) {
        condition_met = qsh_eval_condition(expanded + 3);
        skip_logic = !condition_met;
        continue;
      } else if (strcmp(expanded, "else") == 0) {
        skip_logic = condition_met;
        continue;
      } else if (strcmp(expanded, "fi") == 0) {
        skip_logic = false;
        continue;
      } else if (strcmp(expanded, "then") == 0) {
        continue;
      }

      if (!skip_logic) {
        exstat = execute_single_with_return(expanded);
      }
    }
  }
  return exstat;
}

int cmd_qsh(char *args) {
  if (strlen(args) == 0) {
    printk(qsh_CLR_INFO "BlueOS QSH (qsh) " qsh_CLR_RESET
                        "mode kernel (direct-exec)\n");

    Source stdin_source = {
        .type = SSTDIN, .file = "stdin", .line = 0, .next = source_stack};
    source_stack = &stdin_source;
    qsh_env_t kernel_env = {.type = 1, .oenv = e, .source = &stdin_source};
    e = &kernel_env;
    int res = qsh_shell(&stdin_source);
    e = kernel_env.oenv;
    source_stack = stdin_source.next;
    return res;
  }
  vfs_node_t *node = vfs_findfile(args);
  if (node == NULL) { // Comparamos contra NULL
    printk("qsh: %s: No such file\n", args);
    return 1;
  }

  Source script_source = {.type = SFILE,
                          .file = args,
                          .node = node,
                          .line = 0,
                          .next = source_stack};
  source_stack = &script_source;
  qsh_env_t kernel_env = {.type = 1, .oenv = e, .source = &script_source};
  e = &kernel_env;
  int result = qsh_shell(&script_source);
  e = kernel_env.oenv;
  source_stack = script_source.next;
  return result;
}