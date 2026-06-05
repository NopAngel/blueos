#include <auth.h>
#include <drivers/bluefetch.h>
#include <drivers/connector.h>
#include <drivers/disk.h>
#include <drivers/i2c.h>
#include <drivers/keyboard.h>
#include <drivers/power.h>
#include <fs/ext2.h>
#include <fs/fs.h>
#include <fs/btrfs.h>
#include <fs/vfs.h>
#include <kernel/colors.h>
#include <kernel/commands.h>
#include <kernel/hal.h>
#include <kernel/io.h>
#include <kernel/malloc.h>
#include <kernel/printk.h>
#include <kernel/timer.h>
#include <lib/string.h>
#include <mm/memory.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <version.h>

extern int cmd_qsh(char *args);

// partition_t part_table[MAX_PARTITIONS];
alias_t alias_table[MAX_ALIAS];
unsigned int total_memory_kb = 0;
uint64_t used_memory_kb = 0;
#define LOG_BUFFER_SIZE 4096
#define PRECISION 100
void _blueos_banner();

typedef struct {
  char source_file[32];
  uint32_t offset;
  bool active;
  uint32_t device_id;
} loop_device_t;

loop_device_t loop_devices[4];

/* Global and External Dependencies */
extern void execute_shell_command(char *input);
extern int vmm_map_page(uintptr_t vaddr, uintptr_t paddr, uint32_t flags);
extern uint32_t system_ticks;
extern int fs_needs_sync;
extern uint16_t g_smbus_base;
extern int current_user_index;
extern char current_user[];
extern uint32_t current_dir_cluster;
extern void fat32_ls(uint32_t cluster);
extern void command_pwd();
extern int elf_load(void *elf_data);

typedef struct {
  uint8_t second;
  uint8_t minute;
  uint8_t hour;
  uint8_t day;
  uint8_t month;
  uint32_t year;
} rtc_time_t;
extern void get_local_time(rtc_time_t *time);

extern int cursor_x;
extern int cursor_y;
extern unsigned int directory_count;
extern unsigned int file_count;
extern DirectoryEntry directory_table[];
extern FileEntry file_table[];
extern unsigned int current_directory;

extern char kernel_log_buffer[LOG_BUFFER_SIZE];
extern uint32_t log_ptr;

/* Externs para nuevos comandos */
extern void timer_sleep(uint64_t milliseconds);
typedef struct {
  int index;
  int echo;
  int enabled;
} kbd_state_view_t;
extern kbd_state_view_t kbd_state;
char sys_domainname[32] = "blueos.local";

env_var_t env_vars[MAX_ENV_VARS];

/* History Buffer */
char command_history[HISTORY_MAX][INPUT_BUFFER_SIZE];
int history_count = 0;
int history_current_index = -1;

/* Estados del teclado */
static bool kbd_shift = false;
static bool kbd_caps = false;
static bool kbd_ctrl = false;
static int kbd_layout = 0; // 0 = US, 1 = ES

/* Mapas de teclado: [Layout][Shift][Scancode] */
unsigned char kbd_maps[2][2][128] = {
    { // US Layout
     {// Normal
      0,   27,  '1',  '2',  '3',  '4', '5', '6',  '7', '8', '9', '0',
      '-', '=', '\b', '\t', 'q',  'w', 'e', 'r',  't', 'y', 'u', 'i',
      'o', 'p', '[',  ']',  '\n', 0,   'a', 's',  'd', 'f', 'g', 'h',
      'j', 'k', 'l',  ';',  '\'', '`', 0,   '\\', 'z', 'x', 'c', 'v',
      'b', 'n', 'm',  ',',  '.',  '/', 0,   '*',  0,   ' ', 0},
     {// Shifted
      0,   27,  '!',  '@',  '#',  '$', '%', '^', '&', '*', '(', ')',
      '_', '+', '\b', '\t', 'Q',  'W', 'E', 'R', 'T', 'Y', 'U', 'I',
      'O', 'P', '{',  '}',  '\n', 0,   'A', 'S', 'D', 'F', 'G', 'H',
      'J', 'K', 'L',  ':',  '\"', '~', 0,   '|', 'Z', 'X', 'C', 'V',
      'B', 'N', 'M',  '<',  '>',  '?', 0,   '*', 0,   ' ', 0}},
    { // ES Layout (QWERTY Español)
     {// Normal
      0,    27,  '1',  '2',  '3',  '4', '5', '6', '7', '8', '9', '0',
      '\'', '¡', '\b', '\t', 'q',  'w', 'e', 'r', 't', 'y', 'u', 'i',
      'o',  'p', '`',  '+',  '\n', 0,   'a', 's', 'd', 'f', 'g', 'h',
      'j',  'k', 'l',  'ñ',  '{',  'º', 0,   'ç', 'z', 'x', 'c', 'v',
      'b',  'n', 'm',  ',',  '.',  '-', 0,   '*', 0,   ' ', 0},
     {// Shifted
      0,   27,  '!',  '\"', '·',  '$', '%', '&', '/', '(', ')', '=',
      '?', '¿', '\b', '\t', 'Q',  'W', 'E', 'R', 'T', 'Y', 'U', 'I',
      'O', 'P', '^',  '*',  '\n', 0,   'A', 'S', 'D', 'F', 'G', 'H',
      'J', 'K', 'L',  'Ñ',  '[',  'ª', 0,   'Ç', 'Z', 'X', 'C', 'V',
      'B', 'N', 'M',  ';',  ':',  '_', 0,   '*', 0,   ' ', 0}}};
int history_browse_index = -1;

/* --- Utility Functions --- */

char *get_args(char *input) {
  char *p = input;
  while (*p != ' ' && *p != '\0')
    p++;
  if (*p == ' ') {
    *p = '\0';
    return p + 1;
  }
  return "";
}
char raw_get_char() {
  while (1) {
    if (inb(0x64) & 1) {
      uint8_t scancode = inb(0x60);

      /* Manejo de liberación de teclas (Break codes) */
      if (scancode & 0x80) {
        uint8_t released = scancode & 0x7F;
        if (released == 0x2A || released == 0x36)
          kbd_shift = false;
        if (released == 0x1D)
          kbd_ctrl = false;
        continue;
      }

      /* Manejo de presión de teclas (Make codes) */
      if (scancode == 0x2A || scancode == 0x36) {
        kbd_shift = true;
        continue;
      }
      if (scancode == 0x1D) {
        kbd_ctrl = true;
        continue;
      }
      if (scancode == 0x3A) {
        kbd_caps = !kbd_caps;
        continue;
      }

      /* Shortcuts de Control */
      if (kbd_ctrl) {
        if (scancode == 0x2E)
          return 0x03; // Ctrl+C (ETX)
        if (scancode == 0x2F)
          return 0x16; // Ctrl+V (SYN)
      }

      unsigned char mapped = kbd_maps[kbd_layout][kbd_shift][scancode];

      if (mapped > 0) {
        if (mapped >= 'a' && mapped <= 'z') {
          if (kbd_caps != kbd_shift)
            mapped -= 32; // Convertir a mayúscula
        } else if (mapped >= 'A' &&
                   mapped <= 'Z') { // Para el caso de la Ñ o Ç en el mapa shift
          if (kbd_caps == kbd_shift)
            mapped += 32;
        }
        return (char)mapped;
      }
    }
  }
}

int has_permission(uint32_t file_mode, char mask) {

  if (file_mode == 0)
    return 1;

  if (mask == 'r')
    return (file_mode & 0444);
  if (mask == 'w')
    return (file_mode & 0222);
  if (mask == 'x')
    return (file_mode & 0111);

  return 0;
}

int evaluate_arithmetic(char *exp) {
  long res = 0;
  long num = 0;
  char op = '+';
  int i = 0;

  while (exp[i] != '\0') {
    if (exp[i] == ' ') {
      i++;
      continue;
    }

    if (exp[i] >= '0' && exp[i] <= '9') {
      num = 0;

      while (exp[i] >= '0' && exp[i] <= '9') {
        num = num * 10 + (exp[i] - '0');
        i++;
      }
      num *= PRECISION;

      if (exp[i] == '.') {
        i++;
        int p = PRECISION / 10;
        while (exp[i] >= '0' && exp[i] <= '9' && p > 0) {
          num += (exp[i] - '0') * p;
          p /= 10;
          i++;
        }
        while (exp[i] >= '0' && exp[i] <= '9')
          i++;
      }

      if (op == '+')
        res += num;
      else if (op == '-')
        res -= num;
      else if (op == '*' || op == 'x')
        res = (res * num) / PRECISION;
      else if (op == '/') {
        if (num != 0)
          res = (res * PRECISION) / num;
        else
          printk("[Div/0] ");
      }
      continue;
    }

    if (exp[i] == '+' || exp[i] == '-' || exp[i] == '*' || exp[i] == 'x' ||
        exp[i] == '/') {
      op = exp[i];
    }
    i++;
  }
  return (int)res;
}
void parse_ip(char *str, uint8_t *ip_out) {
  char *ptr = str;
  for (int part = 0; part < 4; part++) {
    ip_out[part] = 0;
    while (*ptr >= '0' && *ptr <= '9') {
      ip_out[part] = ip_out[part] * 10 + (*ptr - '0');
      ptr++;
    }
    if (*ptr == '.')
      ptr++;
  }
}

int copy_file(char *source, char *dest) {
    vfs_node_t* src_node = vfs_findfile(source);
    if (!src_node) return -1;

    // El contenido está guardado en el puntero del nodo
    char *content = (char*)src_node->ptr;

    // Creamos el destino
    if (vfs_touch(dest, content) != 0) return -1;
    
    return 0;
}

int move_file(char *source, char *dest) {
  vfs_node_t* src_node = vfs_findfile(source);

  if (src_node == NULL) {
    printk("ERR: The source '%s' does not exist.\n", source);
    return -1;
  }
  if (vfs_findfile(dest) != NULL) {
    printk("ERR: The destination '%s' already exists.\n", dest);
    return -1;
  }

  // Corregido: Renombramos directamente en el nodo, sin tocar file_table
  strncpy(src_node->name, dest, VFS_NAME_MAX - 1);
  src_node->name[VFS_NAME_MAX - 1] = '\0';

  return 0;
}

/*
int create_partition(char* name, uint32_t start, uint32_t size) {
    for (int i = 0; i < MAX_PARTITIONS; i++) {
        if (!part_table[i].active) {
            strncpy(part_table[i].name, name, 8);
            part_table[i].start_lba = start;
            part_table[i].sectors = size;
            part_table[i].type = 0x83;
            part_table[i].active = true;
            return i;
        }
    }
    return -1;
}
*/
void add_to_history(char *cmd) {
  if (strlen(cmd) == 0)
    return;
  if (history_count > 0 &&
      strcmp(cmd, command_history[(history_count - 1) % HISTORY_MAX]) == 0)
    return;

  strncpy(command_history[history_count % HISTORY_MAX], cmd, INPUT_BUFFER_SIZE);
  history_count++;
  history_current_index = -1;
}

bool match_wildcard(const char *pattern, const char *text) {
  while (*pattern && *text) {
    if (*pattern == '*') {
      pattern++;
      if (!*pattern)
        return true;
      while (*text) {
        if (match_wildcard(pattern, text))
          return true;
        text++;
      }
      return false;
    }
    if (*pattern != *text)
      return false;
    pattern++;
    text++;
  }
  return (*pattern == '*' && *(pattern + 1) == '\0') ||
         (*pattern == '\0' && *text == '\0');
}

void list_items_wildcard(const char *pattern) {
  bool has_wildcard = (pattern && strchr(pattern, '*'));

  for (unsigned int i = 0; i < directory_count; i++) {
    if (directory_table[i].parent_dir == current_directory) {
      if (!has_wildcard || match_wildcard(pattern, directory_table[i].name)) {
        printk("  %s/\n", directory_table[i].name);
      }
    }
  }

  for (unsigned int i = 0; i < file_count; i++) {
    if (file_table[i].parent_dir == current_directory) {
      if (!has_wildcard || match_wildcard(pattern, file_table[i].name)) {
        printk("  %s    ", file_table[i].name);
        printk("%d B\n", file_table[i].size);
      }
    }
  }
}

void fs_rm_wildcard(const char *pattern) {
  if (strchr(pattern, '*') == NULL) {
    vfs_rm(pattern);
    return;
  }

  int deleted_count = 0;
  for (int i = (int)file_count - 1; i >= 0; i--) {
    if (file_table[i].parent_dir == current_directory &&
        match_wildcard(pattern, file_table[i].name)) {

      printk("Deleting: %s...\n", file_table[i].name);

      for (unsigned int j = i; j < file_count - 1; j++) {
        file_table[j] = file_table[j + 1];
      }
      file_count--;
      deleted_count++;
    }
  }

  if (deleted_count > 0)
    printk("Successfully removed %d files.\n", deleted_count);
  else
    printk("No files matched the pattern '%s'.\n", pattern);
}

bool match_pattern(const char *pattern, const char *name) {
  while (*pattern) {
    if (*pattern == '*') {
      if (!*(++pattern))
        return true;
      while (*name) {
        if (match_pattern(pattern, name))
          return true;
        name++;
      }
      return false;
    } else if (*pattern == *name) {
      pattern++;
      name++;
    } else {
      return false;
    }
  }
  return !*pattern && !*name;
}

void set_env_var(char *name, char *value) {
  for (int i = 0; i < MAX_ENV_VARS; i++) {
    if (env_vars[i].active && strcmp(env_vars[i].name, name) == 0) {
      strncpy(env_vars[i].value, value, VAR_VAL_LEN);
      return;
    }
  }
  for (int i = 0; i < MAX_ENV_VARS; i++) {
    if (!env_vars[i].active) {
      strncpy(env_vars[i].name, name, VAR_NAME_LEN);
      strncpy(env_vars[i].value, value, VAR_VAL_LEN);
      env_vars[i].active = true;
      return;
    }
  }
  printk("Error: Environment storage full!\n");
}

char *get_env_var(char *name) {
  for (int i = 0; i < MAX_ENV_VARS; i++) {
    if (env_vars[i].active && strcmp(env_vars[i].name, name) == 0) {
      return env_vars[i].value;
    }
  }
  return "";
}

void resolve_alias(char *input, char *output) {
  for (int i = 0; i < MAX_ALIAS; i++) {
    if (alias_table[i].active && strcmp(input, alias_table[i].name) == 0) {
      strcpy(output, alias_table[i].command);
      return;
    }
  }
  strcpy(output, input);
}

void expand_variables(char *input, char *output) {
  char *src = input;
  char *dest = output;

  while (*src != '\0') {
    if (*src == '$' && *(src + 1) != '\0') {
      src++;
      bool braced = false;
      if (*src == '{') {
        braced = true;
        src++;
      }

      char var_name[VAR_NAME_LEN];
      int i = 0;
      while ((*src >= 'A' && *src <= 'Z') || (*src >= '0' && *src <= '9') ||
             *src == '_') {
        var_name[i++] = *src++;
      }
      var_name[i] = '\0';

      if (braced && *src == '}') {
        src++;
      }

      char *val = get_env_var(var_name);
      while (*val != '\0') {
        *dest++ = *val++;
      }
    } else {
      *dest++ = *src++;
    }
  }
  *dest = '\0';
}

void add_device_to_dev(const char *name, uint8_t major, uint8_t minor,
                       bool block) {
  // touch("/dev/name", [device info]);
  printk("  Found %s: %s device (%d,%d)\n", name, block ? "block" : "char",
         major, minor);
}

void expand_wildcards(char *args, char *out_buffer) {
  if (strchr(args, '*') == NULL) {
    strcpy(out_buffer, args);
    return;
  }

  /*
  foreach(file in current_directory) {
      if (match_pattern(args, file->name)) {
          strcat(out_buffer, file->name);
          strcat(out_buffer, " ");
      }
  }
  */
}

/* --- Command Implementations --- */

int cmd_kbd(char *args) {
  if (strlen(args) == 0) {
    printk("Current layout: %s\n", kbd_layout == 0 ? "US" : "ES");
    return 0;
  }
  if (strcmp(args, "us") == 0)
    kbd_layout = 0;
  else if (strcmp(args, "es") == 0)
    kbd_layout = 1;
  else {
    printk("Usage: kbd <us|es>\n");
    return 1;
  }
  printk("Keyboard layout switched to %s\n", args);
  return 0;
}

int cmd_help(char *args) {
  printk("\nBlueOS Available Commands:\n");
  for (int i = 0; commands_table[i].name != 0; i++) {
    printk("  %s ", commands_table[i].name);
    printk("- %s\n", commands_table[i].description);
  }
}

int cmd_msg(char *args) {
  if (strlen(args) == 0) {
    printk("Use: msg <text for kernel>\n");
    return 1;
  }

  connector_write(args, strlen(args));

  char res[64] = "Message received by BlueOS Core";
  connector_write(res, strlen(res));

  return 0;
}

extern uint16_t g_smbus_base;

#ifdef ARCH_x86
extern uint32_t pci_read_config(uint8_t bus, uint8_t slot, uint8_t func,
                                uint8_t offset);
#endif

int cmd_cat(char *args) {
    if (strlen(args) == 0) {
        printk("Usage: cat <filename>\n");
        return 1;
    }

    // 1. Buscar el archivo
    vfs_node_t *node = vfs_findfile(args);
    if (!node) {
        printk("cat: %s: No such file or directory\n", args);
        return 1;
    }

    // 2. Preparar buffer (usamos el tamaño del archivo)
    uint32_t size = node->size;
    if (size == 0) {
        printk("(archivo vacio)\n");
        return 0;
    }

    char *buffer = (char*)kmalloc(size + 1); // +1 para el terminador nulo
    mm_memset(buffer, 0, size + 1);

    // 3. Leer el contenido
    int bytes_read = vfs_read(node, buffer, size, 0);
    
    if (bytes_read > 0) {
        printk("%s\n", buffer);
    } else {
        printk("cat: error al leer el archivo\n");
    }

    // 4. Liberar memoria
    kfree(buffer);
    
    return 0;
} 

int cmd_main(char *args) { printk("\nTHANKS GOD FOR ALL!!\n"); }

int cmd_flush(char *args) {
  btrfs_flush_cache();
}

int cmd_mv(char *args) {
  if (strlen(args) == 0) {
    printk("Use: mv <origin> <destination>\n");
    return 1;
  }

  char *first_arg = args;
  char *second_arg = strchr(args, ' ');

  if (!second_arg) {
    printk("ERR: The destination is missing.\n");
    return 1;
  }

  *second_arg = '\0';
  second_arg++;
  while (*second_arg == ' ')
    second_arg++;

  if (move_file(first_arg, second_arg) == 0) {
    printk("Moved/Renowned: %s -> %s\n", first_arg, second_arg);
  }

  return 0;
}

int cmd_cp(char *args) {
  if (strlen(args) == 0) {
    printk("Use: cp <origin> <destination>\n");
    return 1;
  }

  char src[32], dest[32];

  char *first_arg = args;
  char *second_arg = strchr(args, ' ');

  if (!second_arg) {
    printk("ERR: The destination name is missing.\n");
    return 1;
  }

  *second_arg = '\0';
  second_arg++;
  while (*second_arg == ' ')
    second_arg++;

  strncpy(src, first_arg, 31);
  strncpy(dest, second_arg, 31);

  if (copy_file(src, dest) == 0) {
    printk("File copied successfully.\n");
  }

  return 0;
}

int cmd_bc(char *args) {
  if (args == NULL || strlen(args) == 0) {
    printk("Error: Use bc <operation> (E.g: bc 5.5x2)\n");
    return 1;
  }

  int result = evaluate_arithmetic(args);

  int integer_part = result / PRECISION;
  int fractional_part = result % PRECISION;
  if (fractional_part < 0)
    fractional_part = -fractional_part;

  if (fractional_part < 10) {
    printk("= %d.0%d\n", integer_part, fractional_part);
  } else {
    printk("= %d.%d\n", integer_part, fractional_part);
  }

  return 0;
}

int cmd_losetup(char *args) {
  if (strlen(args) == 0) {
    printk("Usage: losetup /dev/loopX file_name\n");
    return 1;
  }

  char dev_name[16];
  char file_name[32];

  int i = 0;
  while (args[i] != ' ' && args[i] != '\0' && i < 15) {
    dev_name[i] = args[i];
    i++;
  }
  dev_name[i] = '\0';

  if (args[i] == ' ')
    i++;

  int j = 0;
  while (args[i] != '\0' && j < 31) {
    file_name[j] = args[i];
    i++;
    j++;
  }
  file_name[j] = '\0';
  vfs_node_t* file_node = vfs_findfile(file_name);
  if (file_node == NULL) {
    printk("Error: File '%s' not found.\n", file_name);
    return 1;
  }

  strncpy(loop_devices[0].source_file, file_name, 32);
  loop_devices[0].active = true;

  printk("Success: %s is now mapped to %s\n", file_name, dev_name);
  return 0;
}

int cmd_alias(char *args) {
  if (strlen(args) == 0) {
    printk("\nCurrent Aliases:\n");
    for (int i = 0; i < MAX_ALIAS; i++) {
      if (alias_table[i].active) {
        printk("  alias %s='%s'\n", alias_table[i].name,
               alias_table[i].command);
      }
    }
    return 0;
  }

  char *name = args;
  char *value = strchr(args, '=');

  if (value) {
    *value = '\0';
    value++;

    for (int i = 0; i < MAX_ALIAS; i++) {
      if (!alias_table[i].active || strcmp(alias_table[i].name, name) == 0) {
        strncpy(alias_table[i].name, name, ALIAS_NAME_LEN);
        strncpy(alias_table[i].command, value, ALIAS_CMD_LEN);
        alias_table[i].active = true;
        printk("Alias set: %s -> %s\n", name, value);
        return 0;
      }
    }
  } else {
    printk("Usage: alias name=command\n");
  }
  return 1;
}

int cmd_version(char *args) {
  printk("\nBlueOS Kernel v%s\n", UTS_RELEASE);
  printk("Arch: %s | Compiler: %s\n", BLUEOS_ARCH, COMPILER_INFO);
}

int cmd_clear(char *args) {
  clear_screen();
  cursor_x = 0;
  cursor_y = 0;
}

int cmd_whoami(char *args) {
  if (current_user_index != -1)
    printk("\nYou are: %s\n", users[current_user_index].username);
}

static int vfs_mkdir_recursive(const char *path);

int cmd_ls(char *args) {
  vfs_node_t *node = (strlen(args) > 0) ? vfs_lookup(args) : vfs_get_current();
  if (!node) {
    if (strlen(args) > 0)
      printk("ls: %s: No such file or directory\n", args);
    return 1;
  }

  if (node->type == VFS_TYPE_DIR) {
    struct vfs_dirent dirent;
    int idx = 0;
    while (vfs_readdir(node, idx, &dirent) == 0) {
      printk("%s  ", dirent.name);
      idx++;
    }
    printk("\n");
  } else {
    printk("%s\n", node->name);
  }
  return 0;
}


int cmd_cd(char *args) {
  if (strlen(args) == 0) {
    return vfs_chdir("/") == 0 ? 0 : 1;
  }

  if (vfs_chdir(args) != 0) {
    printk("cd: %s: No such directory\n", args);
    return 1;
  }
  return 0;
}

int cmd_pwd(char *args) {
  char cwd[256] = {0};
  vfs_get_cwd(cwd, sizeof(cwd));
  printk("%s\n", cwd);
  return 0;
}

static int vfs_mkdir_recursive(const char *path) {
  if (!path || *path == '\0')
    return -1;

  char buffer[256];
  strncpy(buffer, path, sizeof(buffer) - 1);
  buffer[sizeof(buffer) - 1] = '\0';

  char *part = strtok(buffer, "/");
  char current_path[256] = {0};
  if (path[0] == '/')
    strcpy(current_path, "/");

  while (part) {
    if (strlen(part) == 0) {
      part = strtok(NULL, "/");
      continue;
    }

    if (strcmp(current_path, "/") == 0) {
      int len = strlen(part);
      if (len + 2 > (int)sizeof(current_path))
        return -1;
      current_path[0] = '/';
      memcpy(current_path + 1, part, len);
      current_path[len + 1] = '\0';
    } else if (current_path[0] == '\0') {
      strncpy(current_path, part, sizeof(current_path) - 1);
      current_path[sizeof(current_path) - 1] = '\0';
    } else {
      int len = strlen(current_path);
      int part_len = strlen(part);
      if (len + 1 + part_len + 1 > (int)sizeof(current_path))
        return -1;
      current_path[len] = '/';
      memcpy(current_path + len + 1, part, part_len);
      current_path[len + 1 + part_len] = '\0';
    }

    if (!vfs_lookup(current_path)) {
      if (vfs_mkdir(current_path) != 0) {
        return -1;
      }
    }
    part = strtok(NULL, "/");
  }
  return 0;
}

int cmd_set(char *args) {
  char *name = args;
  char *value = strchr(args, '=');

  if (value) {
    *value = '\0';
    value++;
    set_env_var(name, value);
    printk("Variable set: %s = %s\n", name, value);
  } else {
    printk("Usage: set NAME=VALUE\n");
  }
}

int cmd_env(char *args) {
  printk("\nEnvironment Variables:\n");
  for (int i = 0; i < MAX_ENV_VARS; i++) {
    if (env_vars[i].active) {
      printk("  %s", env_vars[i].name);
      printk(" = ");
      printk("%s\n", env_vars[i].value);
    }
  }
}

int cmd_mkdir(char *args) {
    // Si tu función vfs_mkdir solo acepta el nombre, llámala así:
    return vfs_mkdir(args); 
}

int cmd_chmod(char *args) {
  if (strlen(args) == 0) {
    printk("Use: chmod <mode> <filename>\nExample: chmod 777 lmfao.txt\n");
    return 1;
  }

  char *mode_str = args;
  char *file_name = strchr(args, ' ');

  if (!file_name) {
    printk("ERR: The file name is missing.\n");
    return 1;
  }

  *file_name = '\0';
  file_name++;
  while (*file_name == ' ')
    file_name++;

  // 1. Buscamos el nodo
  vfs_node_t* node = vfs_findfile(file_name);
  if (node == NULL) {
    printk("ERR: The file '%s' does not exist.\n", file_name);
    return 1;
  }

  int new_mode = simple_strtol(mode_str, NULL, 8);

  // Usamos 'flags' en lugar de 'permissions'
  node->flags = (uint32_t)new_mode; 
  
  printk("Permissions for '%s' updated to %o\n", file_name, new_mode);
  return 0;
}

int cmd_usr(char *args) {
  if (strlen(args) == 0) {
    printk("Usage: usr add <name> <pass>\n");
    return 0;
  }

  char *sub_cmd = args;
  char *next_args = strchr(args, ' ');

  if (next_args) {
    *next_args = '\0';
    next_args++;
  } else {
    printk("Error: Missing sub-command (add).\n");
    return 0;
  }

  if (strcmp(sub_cmd, "add") == 0) {
    char *name = next_args;
    char *pass = strchr(next_args, ' ');

    if (pass) {
      *pass = '\0';
      pass++;
      while (*pass == ' ')
        pass++;

      if (*name != '\0' && *pass != '\0') {
        add_user(name, pass);
      } else {
        printk("Error: Name or password empty.\n");
      }
    } else {
      printk("Error: Usage: usr add <name> <pass>\n");
    }
  } else {
    printk("Error: Unknown sub-command '%s'.\n", sub_cmd);
  }
}

int cmd_touch(char *args) {
  if (strlen(args) > 0) {
    vfs_touch(args, "");
    printk("\nFile '%s' created.\n", args);
  } else
    printk("\nUsage: touch <name>\n");
}

int cmd_rm(char *args) {
  bool recursive = false;
  bool force = false;
  char *path = args;

  if (args[0] == '-') {
    int i = 1;
    while (args[i] != ' ' && args[i] != '\0') {
      if (args[i] == 'r')
        recursive = true;
      if (args[i] == 'f')
        force = true;
      i++;
    }
    path = args + i;
    while (*path == ' ')
      path++;
  }

  if (strlen(path) == 0) {
    printk("rm: missing operand\n");
    return 1;
  }

  if (strchr(path, '*') || strchr(path, '?')) {
    fs_rm_wildcard(path);
    return 0;
  }

  if (strcmp(path, ".") == 0 || strcmp(path, "/") == 0) {
    printk("rm: refusing to remove '%s'\n", path);
    return 1;
  }

  if (vfs_unlink(path) != 0) {
    if (!force) {
      printk("rm: cannot remove '%s'\n", path);
      return 1;
    }
  }

  if (recursive) {
    printk("Note: Recursive removal requested for %s\n", path);
  }
  return 0;
}

int cmd_beep(char *args) {
#ifdef ARCH_x86
  play_sound(750);
  for (volatile int i = 0; i < 20000000; i++)
    ;
  nosound();
#endif
}

int cmd_bluefetch(char *args) { __logo_art_ascii(); }

int cmd_reboot(char *args) { sys_reboot(); }

int cmd_halt(char *args) {
  clear_screen();
  printk("\n  BlueOS has been halted.\n");
  printk("  It is now safe to turn off your computer.\n");
  printk("  Goodbye!\n");

  arch_cpu_halt();

  return 0;
}

int cmd_logout(char *args) {
  current_user_index = -1;
  mm_memset(current_user, 0, 32);
  printk("\nLogged out.\n");
}

int cmd_fdisk(char *args) {
  /*printk("\nDisk /dev/loop0 partitions:\n");
  printk("Device      Boot    Start       End   Sectors  Size  Id Type\n");

  for (int i = 0; i < MAX_PARTITIONS; i++) {
      if (part_table[i].active) {
          uint32_t end = part_table[i].start_lba + part_table[i].sectors - 1;
          uint32_t size_kb = (part_table[i].sectors * 512) / 1024;

          printk("/dev/%-10s  %ld %10ld %10ld %4dK %2x Linux\n",
                 part_table[i].name,
                 part_table[i].start_lba,
                 end,
                 part_table[i].sectors,
                 size_kb,
                 part_table[i].type);
      }
  }
  return 0;*/
}

int cmd_uptime(char *args) {
  uint32_t total_seconds = system_ticks / 100;
  uint32_t hours = total_seconds / 3600;
  uint32_t minutes = (total_seconds % 3600) / 60;
  uint32_t seconds = total_seconds % 60;

  printk(" uptime: %d:%02d:%02d up %d min\n", hours, minutes, seconds, minutes);
  return 0;
}

int cmd_mdev(char *args) {
  if (strcmp(args, "-s") != 0) {
    printk("Usage: mdev -s\n");
    return 1;
  }

  printk("mdev: Scanning for devices...\n");
  add_device_to_dev("sda", 8, 0, true);

  for (int i = 0; i < 4; i++) {
    if (loop_devices[i].active) {
      char name[10] = "loop";
      char num[2];
      itoa(i, num);
      strcat(name, num);

      add_device_to_dev(name, 7, i, true);
    }
  }

  add_device_to_dev("tty0", 4, 0, false);

  return 0;
}

int cmd_free(char *args) {
  uint32_t total_mb = total_memory_kb / 1024;
  uint32_t free_mb = total_mb;

  printk("\n              total        used        free\n");
  printk("RAM usage: [");
  int blocks = 20;
  for (int i = 0; i < blocks; i++) {
    if (i < 2)
      printk("#");
    else
      printk(".");
  }
  printk("]\n\n");

  return 0;
}

int cmd_printf(char *args) {
  if (args == NULL || strlen(args) == 0) {
    return 0;
  }

  for (int i = 0; args[i] != '\0'; i++) {
    if (args[i] == '\\') {
      i++;
      switch (args[i]) {
      case 'n':
        printk("\n");
        break;
      case 't':
        printk("\t");
        break;
      case 'r':
        printk("\r");
        break;
      case '\\':
        printk("\\");
        break;
      case '\"':
        printk("\"");
        break;
      default:
        printk("\\%c", args[i]);
        break;
      }
    } else if (args[i] == '-') {
      i++;
      switch (args[i]) {
      case 'u': {
        extern char current_user[];
        printk("%s", current_user);
      } break;
      case 'n': {
        printk("\n");
      }
      case 't': {
        extern int tty_current();
        printk("tty%d", tty_current());
      } break;
      case '-':
        printk("-");
        break;
      default:
        printk("%%%c", args[i]);
        break;
      }
    } else {
      char buf[2] = {args[i], '\0'};
      printk("%s", buf);
    }
  }

  return 0;
}

int cmd_echo(char *args) {
  if (strlen(args) == 0) {
    printk("\n");
    return 0;
  }
  printk("%s\n", args);
  return 0;
}


int cmd_nproc(char *args) {
  printk("1\n");
  return 0;
}

int cmd_domainname(char *args) {
  if (strlen(args) > 0) {
    strncpy(sys_domainname, args, 31);
  } else {
    printk("%s\n", sys_domainname);
  }
  return 0;
}

int cmd_stty(char *args) {
  if (strcmp(args, "-echo") == 0) {
    kbd_state.echo = 0;
    printk("stty: echo disabled\n");
  } else if (strcmp(args, "echo") == 0) {
    kbd_state.echo = 1;
    printk("stty: echo enabled\n");
  } else {
    printk("speed 38400 baud; line = 0;\n");
    printk("echo = %d\n", kbd_state.echo);
  }
  return 0;
}

int cmd_expr(char *args) {
  if (strlen(args) == 0)
    return 1;
  int res = evaluate_arithmetic(args);
  printk("%d\n", res / PRECISION);
  return 0;
}

int cmd_pwait(char *args) {
  if (strlen(args) == 0)
    return 1;
  printk("Waiting for PID %s...\n", args);
  timer_sleep(500);
  return 0;
}


int cmd_dd(char *args) {
  if (strlen(args) == 0) {
    printk("Usage: dd if=SRC of=DST\n");
    return 1;
  }
  char *if_ptr = strstr(args, "if=");
  char *of_ptr = strstr(args, "of=");
  if (!if_ptr || !of_ptr)
    return 1;

  char src[32], dst[32];

  /* Extracción manual de parámetros (sustituye a sscanf) */
  char *s = if_ptr + 3;
  int k = 0;
  while (*s != ' ' && *s != '\0' && k < 31)
    src[k++] = *s++;
  src[k] = '\0';

  char *d = of_ptr + 3;
  k = 0;
  while (*d != ' ' && *d != '\0' && k < 31)
    dst[k++] = *d++;
  dst[k] = '\0';

  if (copy_file(src, dst) == 0) {
    printk("Records in/out copied.\n");
  }
  return 0;
}

int cmd_ascii(char *args) {
  printk("\n--- ASCII Table (Hex:Dec:Char) ---\n");
  for (int i = 32; i < 127; i++) {
    char s[2] = {(char)i, '\0'};
    printk(" %x:%d:%s ", i, i, s);
    if ((i - 31) % 6 == 0)
      printk("\n");
  }
  printk("\n");
  return 0;
}

int cmd_cal(char *args) {
  rtc_time_t now;
  get_local_time(&now);

  int month = now.month;
  int year = now.year;

  /* Si el usuario pasa argumentos, podríamos parsearlos con simple_strtol */

  const char *months[] = {
      "",     "January", "February",  "March",   "April",    "May",     "June",
      "July", "August",  "September", "October", "November", "December"};

  int days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  /* Ajuste para año bisiesto */
  if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
    days_in_month[2] = 29;
  }

  printk("\n    %s %d\n", months[month], year);
  printk(" Su Mo Tu We Th Fr Sa\n");

  /* Algoritmo de Sakamoto para encontrar el primer día de la semana */
  int y = year - (14 - month) / 12;
  int x = y + y / 4 - y / 100 + y / 400;
  int m = month + 12 * ((14 - month) / 12) - 2;
  int first_day = (1 + x + (31 * m) / 12) % 7;

  /* Espacios iniciales */
  for (int i = 0; i < first_day; i++) {
    printk("   ");
  }

  for (int day = 1; day <= days_in_month[month]; day++) {
    /* Resaltar el día actual */
    if (day == now.day) {
      printk("\033[32m%2d\033[0m ", day);
    } else {
      printk("%2d ", day);
    }

    if ((day + first_day) % 7 == 0) {
      printk("\n");
    }
  }
  printk("\n");
  return 0;
}

int cmd_run(char *args) {
  if (strlen(args) == 0) {
    printk("Usage: run <elf_file>\n");
    return 1;
  }

  vfs_node_t *node = vfs_lookup(args);
  if (!node || node->type != VFS_TYPE_FILE) {
    printk("run: %s not found or is not a file\n", args);
    return 1;
  }

  void *buffer = kmalloc(node->size);
  if (!buffer) {
    printk("run: Out of memory\n");
    return 1;
  }

  vfs_read(node, buffer, node->size, 0);

  printk("Executing ELF: %s...\n", args);
  elf_load(buffer);

  kfree(buffer);
  return 0;
}

static void tree_helper(vfs_node_t *node, char *prefix) {
  struct vfs_dirent dirent;
  int i = 0;
  int total = 0;

  /* Contamos el total de entradas para identificar la última */
  while (vfs_readdir(node, total, &dirent) == 0)
    total++;

  while (vfs_readdir(node, i, &dirent) == 0) {
    /* Saltamos las entradas de navegación para evitar bucles o ruido visual */
    if (strcmp(dirent.name, ".") == 0 || strcmp(dirent.name, "..") == 0) {
      i++;
      continue;
    }

    bool is_last = (i == total - 1);
    printk("%s%c-- %s%s\n", prefix, is_last ? '`' : '|', dirent.name,
           (dirent.type == VFS_TYPE_DIR) ? "/" : "");

    if (dirent.type == VFS_TYPE_DIR) {
      vfs_node_t *child = vfs_finddir(node, dirent.name);
      if (child) {
        char next_prefix[128];
        strcpy(next_prefix, prefix);
        strcat(next_prefix, is_last ? "    " : "|   ");
        tree_helper(child, next_prefix);
      }
    }
    i++;
  }
}

int cmd_tree(char *args) {
  char *path = (strlen(args) > 0) ? args : ".";
  vfs_node_t *root =
      (strcmp(path, ".") == 0) ? vfs_get_current() : vfs_lookup(path);

  if (!root || root->type != VFS_TYPE_DIR) {
    printk("tree: %s: No such directory\n", path);
    return 1;
  }

  printk("%s\n", (strcmp(path, ".") == 0) ? "." : path);
  char prefix[128] = {0};
  tree_helper(root, prefix);
  return 0;
}

int cmd_ed(char *args) {
  if (strlen(args) == 0) {
    printk("ed: Filename required\n");
    return 1;
  }
  char buffer[1024] = {0};
  printk("Entering ed (Line Editor). Type '.' on a new line to save.\n");

  while (1) {
    char line[64];
    int idx = 0;
    printk("> ");
    while (idx < 63) {
      char c = raw_get_char();
      if (c == '\n') {
        line[idx] = '\0';
        printk("\n");
        break;
      }
      if (c == '\b' && idx > 0) {
        idx--;
        printk("\b \b");
      } else if (c >= 32 && c <= 126) {
        line[idx++] = c;
        char s[2] = {c, '\0'};
        printk(s);
      }
      for (volatile int d = 0; d < 100000; d++)
        ;
    }

    if (strcmp(line, ".") == 0)
      break;
    strcat(buffer, line);
    strcat(buffer, "\n");
  }
  if (vfs_touch(args, buffer) != 0) {
    printk("Saved failed: could not write %s\n", args);
    return 1;
  }
  printk("Saved to %s\n", args);
  return 0;
}

/* --- Autocomplete Logic --- */

void list_matches(char *prefix) {
  int len = strlen(prefix);
  printk("\nPossible commands:\n");
  int count = 0;
  for (int i = 0; commands_table[i].name != 0; i++) {
    if (strncmp(prefix, commands_table[i].name, len) == 0) {
      printk("  %s", commands_table[i].name);
      count++;
      if (count % 4 == 0)
        printk("\n");
    }
  }
  printk("\n");
}

/* --- Commands Dispatch Table --- */

shell_command_t commands_table[] = {
    {"main", "THANKS YOU GOD FOR EVERTHING!", cmd_main},
    {"help", "Show this help menu", cmd_help},
    {"echo", "Show a message", cmd_echo},
    {"version", "Show system version", cmd_version},
    {"clear", "Clear the screen", cmd_clear},
    {"whoami", "Show current user", cmd_whoami},
    {"ls", "List files", cmd_ls},
    {"cd", "Change directory", cmd_cd},
    {"pwd", "Print working directory", cmd_pwd},
    {"mkdir", "Create directory", cmd_mkdir},
    {"touch", "Create empty file", cmd_touch},
    {"usr", "User management (add <name> <pass>)", cmd_usr},
    {"cat", "Read file content", cmd_cat},
    {"mdev", "Device node manager (mdev -s)", cmd_mdev},
    {"rm", "Delete file", cmd_rm},
    {"uptime", "Shows the current date", cmd_uptime},
    {"bluefetch", "System information", cmd_bluefetch},
    {"logout", "Close session", cmd_logout},
    {"msg", "Send a message to the kernel through the connector", cmd_msg},
    {"halt", "It stops the system safely", cmd_halt},
    {"reboot", "Restart BlueOS", cmd_reboot},
    {"set", "Define a variable", cmd_set},
    {"alias", "Defines or lists command aliases", cmd_alias},
    {"losetup", "Sets and controls loop devices", cmd_losetup},
    {"free", "It shows the amount of free and used memory", cmd_free},
    {"env", "List all variables", cmd_env},
    {"bc", "Basic Calculator", cmd_bc},
    {"cp", "Copy files or directories", cmd_cp},
    {"mv", "Move or rename files/directories", cmd_mv},
    {"chmod", "It allows you to set permissions for directories/files",
     cmd_chmod},
    {"qsh", "QSH, Bash alternative", cmd_qsh},
    {"printf",
     "Print formatted text (supports \\n, \\t, %u for user, %t for tty)",
     cmd_printf},
    {"stty", "Change and print terminal line settings", cmd_stty},
    {"pwait", "Wait for process termination", cmd_pwait},
    {"nproc", "Print the number of processing units available", cmd_nproc},
    {"expr", "Evaluate expressions", cmd_expr},
    {"ed", "The standard line editor", cmd_ed},
    {"domainname", "Show or set the system's NIS/YP domain name",
     cmd_domainname},
    {"dd", "Convert and copy a file", cmd_dd},
    {"cal", "Display a calendar", cmd_cal},
    {"ascii", "Show ASCII table", cmd_ascii},
    {"tree", "Display directory structure as a tree", cmd_tree},
    {"run", "Execute an ELF binary from VFS", cmd_run},
    {"kbd", "Switch keyboard layout (us/es)", cmd_kbd},

    {0, 0, 0}};

/* --- Shell Engine --- */

void print_prompt() {
  bool any_active = false;
  for (int i = 0; i < MAX_USERS; i++) {
    if (users[i].active) {
      any_active = true;
      break;
    }
  }

  if (current_user_index != -1) {
    printk("%s@blueos", users[current_user_index].username);
    printk(":");
    printk("%s", users[current_user_index].cwd);
    printk("$ ");
  } else if (!any_active) {
    printk("anon@blueos:/$ ");
  } else {
    printk("blueos login: ");
  }
}

int execute_single_with_return(char *input) {
  char expanded[INPUT_BUFFER_SIZE];
  expand_variables(input, expanded);

  char *args = get_args(expanded);
  for (int i = 0; commands_table[i].name != 0; i++) {
    if (strcmp(expanded, commands_table[i].name) == 0) {
      return commands_table[i].function(args); // return 0 or 1
    }
  }
  printk("Unknown command: %s\n", expanded);
  return 1;
}

void execute_shell_command(char *input) {
  char resolved[INPUT_BUFFER_SIZE];
  resolve_alias(input, resolved);
  if (strlen(input) == 0) {
    return;
  }

  bool any_active = false;
  for (int i = 0; i < MAX_USERS; i++) {
    if (users[i].active) {
      any_active = true;
      break;
    }
  }

  if (any_active && current_user_index == -1) {
    if (strcmp(input, "login") == 0) {
      char temp_user[32];
      char temp_pass[32];
      int i;
      char c;

      printk("Username: ");
      i = 0;
      while (i < 31) {
        c = raw_get_char();
        if (c == '\n') {
          temp_user[i] = '\0';
          printk("\n");
          break;
        } else if (c == '\b' && i > 0) {
          i--;
          printk("\b \b");
        } else if (c >= 32 && c <= 126) {
          temp_user[i++] = c;
          char str[2] = {c, '\0'};
          printk(str);
        }
        for (volatile int d = 0; d < 100000; d++)
          ;
      }

      printk("Password: ");
      i = 0;
      while (i < 31) {
        c = raw_get_char();
        if (c == '\n') {
          temp_pass[i] = '\0';
          printk("\n");
          break;
        } else if (c == '\b' && i > 0) {
          i--;
          printk("\b \b");
        } else if (c >= 32 && c <= 126) {
          temp_pass[i++] = c;
          printk("*");
        }
        for (volatile int d = 0; d < 100000; d++)
          ;
      }

      if (check_login(temp_user, temp_pass)) {
        strncpy(current_user, temp_user, 31);
        current_user[31] = '\0';
        clear_screen();
        printk("Welcome to BlueOS, %s!\n", current_user);
        printk(
            " Copyright (c) 2025,2025             GPL-3.0\n @NopAngel\nFor "
            "commands, use 'help', it basically explains how to use them.\n");

      } else {

        printk("Login incorrect.\n");
      }
    } else {
      printk("Please type 'login' to continue.\n");
    }

    return;
  }

  char *next_ptr = input;
  char *token;

  while ((token = strstr(next_ptr, "&&")) != NULL) {
    *token = '\0';

    int result = execute_single_with_return(next_ptr);

    if (result != 0) {
      print_prompt();
      return;
    }

    next_ptr = token + 2;
    while (*next_ptr == ' ')
      next_ptr++;
  }

  execute_single_with_return(next_ptr);
}
