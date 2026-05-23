#include <lib/string.h>
#include <drivers/connector.h>
#include <drivers/i2c.h>
#include <stdint.h>
#include <kernel/printk.h>
#include <kernel/timer.h>
#include <stddef.h>
#include <kernel/colors.h>
#include <kernel/commands.h>
#include <drivers/keyboard.h>
#include <fs/fs.h>
#include <kernel/io.h>
#include <fs/vfs.h>
#include <version.h>
#include <lib/string.h>
#include <mm/memory.h>
#include <drivers/power.h>
#include <kernel/hal.h>
#include <drivers/bluefetch.h>
#include <auth.h>
#include <stdint.h>
#include <stdbool.h>


//partition_t part_table[MAX_PARTITIONS];
alias_t alias_table[MAX_ALIAS];
int total_memory_kb;
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
extern int vmm_map_page(uintptr_t vaddr, uintptr_t paddr, uint32_t flags);
extern uint32_t system_ticks;
extern int fs_needs_sync;
extern uint16_t g_smbus_base;
extern int current_user_index;
extern char current_user[];
extern uint32_t current_dir_cluster;
extern void fat32_ls(uint32_t cluster);
extern void command_pwd();
extern int cursor_x;
extern int cursor_y;
extern unsigned int directory_count;
extern unsigned int file_count;
extern DirectoryEntry directory_table[];
extern FileEntry file_table[];
extern unsigned int current_directory;

extern char kernel_log_buffer[LOG_BUFFER_SIZE];
extern uint32_t log_ptr;

env_var_t env_vars[MAX_ENV_VARS];

/* History Buffer */
char command_history[HISTORY_MAX][INPUT_BUFFER_SIZE];
int history_count = 0;
int history_current_index = -1;
int history_browse_index = -1;

/* --- Utility Functions --- */

char* get_args(char* input) {
    char* p = input;
    while (*p != ' ' && *p != '\0') p++;
    if (*p == ' ') {
        *p = '\0';
        return p + 1;
    }
    return "";
}
unsigned char kbd_us[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0
};

char raw_get_char() {
    char c = 0;
    while (1) {
        if (inb(0x64) & 1) {
            uint8_t scancode = inb(0x60);
            if (!(scancode & 0x80)) {
                c = kbd_us[scancode];
                if (c > 0) return c;
            }
        }
    }
}

int has_permission(uint32_t file_mode, char mask) {

    if (file_mode == 0) return 1;

    if (mask == 'r') return (file_mode & 0444);
    if (mask == 'w') return (file_mode & 0222);
    if (mask == 'x') return (file_mode & 0111);

    return 0;
}

int evaluate_arithmetic(char* exp) {
    long res = 0;
    long num = 0;
    char op = '+';
    int i = 0;

    while (exp[i] != '\0') {
        if (exp[i] == ' ') { i++; continue; }

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
                while (exp[i] >= '0' && exp[i] <= '9') i++;
            }

            if (op == '+') res += num;
            else if (op == '-') res -= num;
            else if (op == '*' || op == 'x') res = (res * num) / PRECISION;
            else if (op == '/') {
                if (num != 0) res = (res * PRECISION) / num;
                else printk(RED, "[Div/0] ");
            }
            continue;
        }

        if (exp[i] == '+' || exp[i] == '-' || exp[i] == '*' || exp[i] == 'x' || exp[i] == '/') {
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
        if (*ptr == '.') ptr++;
    }
}


int copy_file(char* source, char* dest) {
    int src_idx = find_file(source);
    if (src_idx == -1) {
        printk(RED, "ERR: Source file '%s' does not exist.\n", source);
        return -1;
    }

    char* content = file_table[src_idx].content;
    uint32_t size = file_table[src_idx].size;

    if (find_file(dest) != -1) {
        printk(YELLOW, "WARN: Destiny already exists. Overwriting...\n");
    }

    touch(dest, content);
    return 0;
}


int move_file(char* source, char* dest) {
    int src_idx = find_file(source);
    if (src_idx == -1) {
        printk(RED, "ERR: The source '%s' does not exist.\n", source);
        return -1;
    }
    if (find_file(dest) != -1) {
        printk(RED, "ERR: The destination '%s' already exists.\n", dest);
        return -1;
    }



    strncpy(file_table[src_idx].name, dest, 31);
    file_table[src_idx].name[31] = '\0';

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
void add_to_history(char* cmd) {
    if (strlen(cmd) == 0) return;
    if (history_count > 0 && strcmp(cmd, command_history[(history_count - 1) % HISTORY_MAX]) == 0) return;

    strncpy(command_history[history_count % HISTORY_MAX], cmd, INPUT_BUFFER_SIZE);
    history_count++;
    history_current_index = -1;
}



bool match_wildcard(const char *pattern, const char *text) {
    while (*pattern && *text) {
        if (*pattern == '*') {
            pattern++;
            if (!*pattern) return true;
            while (*text) {
                if (match_wildcard(pattern, text)) return true;
                text++;
            }
            return false;
        }
        if (*pattern != *text) return false;
        pattern++;
        text++;
    }
    return (*pattern == '*' && *(pattern + 1) == '\0') || (*pattern == '\0' && *text == '\0');
}


void list_items_wildcard(const char *pattern) {
    bool has_wildcard = (pattern && strchr(pattern, '*'));

    for (unsigned int i = 0; i < directory_count; i++) {
        if (directory_table[i].parent_dir == current_directory) {
            if (!has_wildcard || match_wildcard(pattern, directory_table[i].name)) {
                printk(CYAN, "  %s/\n", directory_table[i].name);
            }
        }
    }

    for (unsigned int i = 0; i < file_count; i++) {
        if (file_table[i].parent_dir == current_directory) {
            if (!has_wildcard || match_wildcard(pattern, file_table[i].name)) {
                printk(WHITE, "  %s    ", file_table[i].name);
                printk(YELLOW, "%d B\n", file_table[i].size);
            }
        }
    }
}

void fs_rm_wildcard(const char *pattern) {
    if (strchr(pattern, '*') == NULL) {
        fs_rm(pattern);
        return;
    }

    int deleted_count = 0;
    for (int i = (int)file_count - 1; i >= 0; i--) {
        if (file_table[i].parent_dir == current_directory &&
            match_wildcard(pattern, file_table[i].name)) {

            printk(GRAY, "Deleting: %s...\n", file_table[i].name);

            for (unsigned int j = i; j < file_count - 1; j++) {
                file_table[j] = file_table[j + 1];
            }
            file_count--;
            deleted_count++;
        }
    }

    if (deleted_count > 0) printk(GREEN, "Successfully removed %d files.\n", deleted_count);
    else printk(RED, "No files matched the pattern '%s'.\n", pattern);
}

bool match_pattern(const char *pattern, const char *name) {
    while (*pattern) {
        if (*pattern == '*') {
            if (!*(++pattern)) return true;
            while (*name) {
                if (match_pattern(pattern, name)) return true;
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

void set_env_var(char* name, char* value) {
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
    printk(RED, "Error: Environment storage full!\n");
}

char* get_env_var(char* name) {
    for (int i = 0; i < MAX_ENV_VARS; i++) {
        if (env_vars[i].active && strcmp(env_vars[i].name, name) == 0) {
            return env_vars[i].value;
        }
    }
    return "";
}

void resolve_alias(char* input, char* output) {
    for (int i = 0; i < MAX_ALIAS; i++) {
        if (alias_table[i].active && strcmp(input, alias_table[i].name) == 0) {
            strcpy(output, alias_table[i].command);
            return;
        }
    }
    strcpy(output, input);
}

void expand_variables(char* input, char* output) {
    char* src = input;
    char* dest = output;

    while (*src != '\0') {
        if (*src == '$') {
            src++;
            char var_name[VAR_NAME_LEN];
            int i = 0;
            while ((*src >= 'A' && *src <= 'Z') || (*src >= '0' && *src <= '9') || *src == '_') {
                var_name[i++] = *src++;
            }
            var_name[i] = '\0';

            char* val = get_env_var(var_name);
            while (*val != '\0') {
                *dest++ = *val++;
            }
        } else {
            *dest++ = *src++;
        }
    }
    *dest = '\0';
}


void add_device_to_dev(const char* name, uint8_t major, uint8_t minor, bool block) {
    // touch("/dev/name", [device info]);
    printk(GRAY, "  Found %s: %s device (%d,%d)\n",
           name, block ? "block" : "char", major, minor);
}

void expand_wildcards(char* args, char* out_buffer) {
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

int cmd_help(char* args) {
    printk(WHITE, "\nBlueOS Available Commands:\n");
    for (int i = 0; commands_table[i].name != 0; i++) {
        printk(CYAN, "  %s ", commands_table[i].name);
        printk(GRAY, "- %s\n", commands_table[i].description);
    }
}


int cmd_msg(char* args) {
    if (strlen(args) == 0) {
        printk(RED, "Use: msg <text for kernel>\n");
        return 1;
    }

    connector_write(args, strlen(args));

    char res[64] = "Message received by BlueOS Core";
    connector_write(res, strlen(res));

    return 0;
}

extern uint16_t g_smbus_base;


#ifdef ARCH_x86
	extern uint32_t pci_read_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
#endif


int cmd_cat(char* args) {
    if (strlen(args) == 0) {
        printk(RED, "\nUse: cat <file>\n");
        return 1;
    }

    int idx = find_file(args);
    if (idx == -1) {
        printk(RED, "\nERR: The file '%s' does not exist.\n", args);
        return 1;
    }

    if (!has_permission(file_table[idx].permissions, 'r')) {
        printk(RED, "\nAccess denied: You do not have read permissions (r)\n");
        return 1;
    }

    printk(WHITE, "\n%s\n", file_table[idx].content);
    return 0;
}

int cmd_main(char* args) {
    printk(GREEN, "\nTHANKS GOD FOR ALL!!\n");
}

int cmd_mv(char* args) {
    if (strlen(args) == 0) {
        printk(RED, "Use: mv <origin> <destination>\n");
        return 1;
    }

    char* first_arg = args;
    char* second_arg = strchr(args, ' ');

    if (!second_arg) {
        printk(RED, "ERR: The destination is missing.\n");
        return 1;
    }

    *second_arg = '\0';
    second_arg++;
    while (*second_arg == ' ') second_arg++;

    if (move_file(first_arg, second_arg) == 0) {
        printk(GREEN, "Moved/Renowned: %s -> %s\n", first_arg, second_arg);
    }

    return 0;
}


int cmd_cp(char* args) {
    if (strlen(args) == 0) {
        printk(RED, "Use: cp <origin> <destination>\n");
        return 1;
    }

    char src[32], dest[32];

    char* first_arg = args;
    char* second_arg = strchr(args, ' ');

    if (!second_arg) {
        printk(RED, "ERR: The destination name is missing.\n");
        return 1;
    }

    *second_arg = '\0';
    second_arg++;
    while (*second_arg == ' ') second_arg++;

    strncpy(src, first_arg, 31);
    strncpy(dest, second_arg, 31);

    if (copy_file(src, dest) == 0) {
        printk(GREEN, "File copied successfully.\n");
    }

    return 0;
}

int cmd_bc(char* args) {
    if (args == NULL || strlen(args) == 0) {
        printk(RED, "Error: Use bc <operation> (E.g: bc 5.5x2)\n");
        return 1;
    }

    int result = evaluate_arithmetic(args);

    int integer_part = result / PRECISION;
    int fractional_part = result % PRECISION;
    if (fractional_part < 0) fractional_part = -fractional_part;

    if (fractional_part < 10) {
        printk(GREEN, "= %d.0%d\n", integer_part, fractional_part);
    } else {
        printk(GREEN, "= %d.%d\n", integer_part, fractional_part);
    }

    return 0;
}

int cmd_losetup(char* args) {
    if (strlen(args) == 0) {
        printk(RED, "Usage: losetup /dev/loopX file_name\n");
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

    if (args[i] == ' ') i++;

    int j = 0;
    while (args[i] != '\0' && j < 31) {
        file_name[j] = args[i];
        i++;
        j++;
    }
    file_name[j] = '\0';
    int file_idx = find_file(file_name);
    if (file_idx == -1) {
        printk(RED, "Error: File '%s' not found.\n", file_name);
        return 1;
    }

    strncpy(loop_devices[0].source_file, file_name, 32);
    loop_devices[0].active = true;

    printk(GREEN, "Success: %s is now mapped to %s\n", file_name, dev_name);
    return 0;
}

int cmd_alias(char* args) {
    if (strlen(args) == 0) {
        printk(CYAN, "\nCurrent Aliases:\n");
        for (int i = 0; i < MAX_ALIAS; i++) {
            if (alias_table[i].active) {
                printk(WHITE, "  alias %s='%s'\n", alias_table[i].name, alias_table[i].command);
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
                printk(GREEN, "Alias set: %s -> %s\n", name, value);
                return 0;
            }
        }
    } else {
        printk(RED, "Usage: alias name=command\n");
    }
    return 1;
}

int cmd_version(char* args) {
    printk(CYAN, "\nBlueOS Kernel v%s\n", UTS_RELEASE);
    printk(WHITE, "Arch: %s | Compiler: %s\n", BLUEOS_ARCH, COMPILER_INFO);
}

int cmd_clear(char* args) {
    clear_screen();
    cursor_x = 0; cursor_y = 0;

}


int cmd_whoami(char* args) {
    if (current_user_index != -1)
        printk(CYAN, "\nYou are: %s\n", users[current_user_index].username);
}

int cmd_ls(char* args) {
    printk(WHITE, "\n");
    list_items();
}


int cmd_cd(char* args) {
    if (strlen(args) > 0) cd(args);
}

int cmd_pwd(char* args) {
    pwd();
}

int cmd_set(char* args) {
    char *name = args;
    char *value = strchr(args, '=');

    if (value) {
        *value = '\0';
        value++;
        set_env_var(name, value);
        printk(GREEN, "Variable set: %s = %s\n", name, value);
    } else {
        printk(RED, "Usage: set NAME=VALUE\n");
    }
}

int cmd_env(char* args) {
    printk(CYAN, "\nEnvironment Variables:\n");
    for (int i = 0; i < MAX_ENV_VARS; i++) {
        if (env_vars[i].active) {
            printk(WHITE, "  %s", env_vars[i].name);
            printk(GRAY, " = ");
            printk(WHITE, "%s\n", env_vars[i].value);
        }
    }
}

int cmd_mkdir(char* args) {
    if (strlen(args) > 0) mkdir(args);
    else printk(RED, "\nUsage: mkdir <name>\n");
}


int cmd_chmod(char* args) {
    if (strlen(args) == 0) {
        printk(RED, "Use");
        printk(GRAY, "Example: chmod 777 lmfao.txt\n");
        return 1;
    }

    char* mode_str = args;
    char* file_name = strchr(args, ' ');

    if (!file_name) {
        printk(RED, "ERR: The file name is missing.\n");
        return 1;
    }

    *file_name = '\0';
    file_name++;
    while (*file_name == ' ') file_name++;

    int idx = find_file(file_name);
    if (idx == -1) {
        printk(RED, "ERR: The file '%s' does not exist.\n", file_name);
        return 1;
    }

    int new_mode = simple_strtol(mode_str, NULL, 8);

    file_table[idx].permissions = new_mode;
    printk(GREEN, "Permissions for '%s' updated to %s\n", file_name, mode_str);

    return 0;
}


int cmd_usr(char* args) {
    if (strlen(args) == 0) {
        printk(RED, "Usage: usr add <name> <pass>\n");
        return 0;
    }

    char *sub_cmd = args;
    char *next_args = strchr(args, ' ');

    if (next_args) {
        *next_args = '\0';
        next_args++;
    } else {
        printk(RED, "Error: Missing sub-command (add).\n");
        return 0;
    }


    if (strcmp(sub_cmd, "add") == 0) {
        char *name = next_args;
        char *pass = strchr(next_args, ' ');

        if (pass) {
            *pass = '\0';
            pass++;
            while (*pass == ' ') pass++;

            if (*name != '\0' && *pass != '\0') {
                add_user(name, pass);
            } else {
                printk(RED, "Error: Name or password empty.\n");
            }
        } else {
            printk(RED, "Error: Usage: usr add <name> <pass>\n");
        }
    } else {
        printk(RED, "Error: Unknown sub-command '%s'.\n", sub_cmd);
    }
}

int cmd_touch(char* args) {
    if (strlen(args) > 0) {
        touch(args, "");
        printk(GREEN, "\nFile '%s' created.\n", args);
    } else printk(RED, "\nUsage: touch <name>\n");
}





int cmd_rm(char* args) {
    if (strlen(args) == 0) return 1;

    int idx = find_file(args);
    if (idx != -1) {
        if (!has_permission(file_table[idx].permissions, 'w')) {
            printk(RED, "Error: You do not have write permission (w) to delete '%s'.\n", args);
            return 1;
        }
    }

    fs_rm_wildcard(args);
    return 0;
}

int cmd_beep(char* args) {
   #ifdef ARCH_x86
    play_sound(750);
    for(volatile int i=0; i<20000000; i++);
    nosound();
   #endif
}

int cmd_bluefetch(char* args) {
    __logo_art_ascii();
}

int cmd_reboot(char* args) {
    sys_reboot();
}

int cmd_halt(char* args) {
    clear_screen();
    printk(RED, "\n  BlueOS has been halted.\n");
    printk(WHITE, "  It is now safe to turn off your computer.\n");
    printk(GRAY, "  Goodbye!\n");

    arch_cpu_halt();

    return 0;
}

int cmd_logout(char* args) {
    current_user_index = -1;
    mm_memset(current_user, 0, 32);
    printk(YELLOW, "\nLogged out.\n");
}


int cmd_fdisk(char* args) {
    /*printk(CYAN, "\nDisk /dev/loop0 partitions:\n");
    printk(WHITE, "Device      Boot    Start       End   Sectors  Size  Id Type\n");

    for (int i = 0; i < MAX_PARTITIONS; i++) {
        if (part_table[i].active) {
            uint32_t end = part_table[i].start_lba + part_table[i].sectors - 1;
            uint32_t size_kb = (part_table[i].sectors * 512) / 1024;

            printk(WHITE, "/dev/%-10s  %ld %10ld %10ld %4dK %2x Linux\n",
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


int cmd_uptime(char* args) {
    uint32_t total_seconds = system_ticks / 100;
    uint32_t hours = total_seconds / 3600;
    uint32_t minutes = (total_seconds % 3600) / 60;
    uint32_t seconds = total_seconds % 60;

    printk(WHITE, " uptime: %d:%02d:%02d up %d min\n", hours, minutes, seconds, minutes);
    return 0;
}

int cmd_mdev(char* args) {
    if (strcmp(args, "-s") != 0) {
        printk(WHITE, "Usage: mdev -s\n");
        return 1;
    }

    printk(CYAN, "mdev: Scanning for devices...\n");
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



int cmd_free(char* args) {
    uint32_t used_memory_kb = 0;


    uint32_t total_mb = total_memory_kb / 1024;
    uint32_t free_mb = total_mb;

    printk(CYAN, "\n              total        used        free\n");
    printk(GRAY, "RAM usage: [");
    int blocks = 20;
    for(int i=0; i<blocks; i++) {
        if(i < 2) printk(GREEN, "#");
        else printk(GRAY, ".");
    }
    printk(GRAY, "]\n\n");

    return 0;
}


int cmd_printf(char* args) {
    if (args == NULL || strlen(args) == 0) {
        return 0; 
    }



    for (int i = 0; args[i] != '\0'; i++) {
        if (args[i] == '\\') {
            i++;
            switch (args[i]) {
                case 'n': printk(WHITE, "\n"); break;
                case 't': printk(WHITE, "\t"); break;
                case 'r': printk(WHITE, "\r"); break;
                case '\\': printk(WHITE, "\\"); break;
                case '\"': printk(WHITE, "\""); break;
                default:  printk(WHITE, "\\%c", args[i]); break;
            }
        } 
        else if (args[i] == '-') {
            i++;
            switch (args[i]) {
                case 'u': 
                    {
                        extern char current_user[];
                        printk(CYAN, "%s", current_user);
                    }
                    break;
                case 'n':
                    {
                        printk(WHITE, "\n");
                    }
                case 't': 
                    {
                        extern int tty_current();
                        printk(YELLOW, "tty%d", tty_current());
                    }
                    break;
                case '-': 
                    printk(WHITE, "-"); 
                    break;
                default:
                    printk(WHITE, "%%%c", args[i]);
                    break;
            }
        }
        else {
            char buf[2] = {args[i], '\0'};
            printk(WHITE, "%s", buf);
        }
    }

    return 0;
}

int cmd_echo(char* args) {
    if (strlen(args) == 0) {
        printk(WHITE, "\n");
        return 0;
    }
    printk(WHITE, "%s\n", args);
    return 0;
}

/* --- Autocomplete Logic --- */

void list_matches(char* prefix) {
    int len = strlen(prefix);
    printk(GRAY, "\nPossible commands:\n");
    int count = 0;
    for (int i = 0; commands_table[i].name != 0; i++) {
        if (strncmp(prefix, commands_table[i].name, len) == 0) {
            printk(CYAN, "  %s", commands_table[i].name);
            count++;
            if (count % 4 == 0) printk(WHITE, "\n");
        }
    }
    printk(WHITE, "\n");
}

/* --- Commands Dispatch Table --- */

shell_command_t commands_table[] = {
    {"main",      "Divine gratitude",                                          cmd_main},
    {"help",      "Show this help menu",                                       cmd_help},
    {"echo",      "Show a message",                                            cmd_echo},
    {"version",   "Show system version",                                       cmd_version},
    {"clear",     "Clear the screen",                                          cmd_clear},
    {"whoami",    "Show current user",                                         cmd_whoami},
    {"ls",        "List files",                                                cmd_ls},
    {"cd",        "Change directory",                                          cmd_cd},
    {"pwd",       "Print working directory",                                   cmd_pwd},
    {"mkdir",     "Create directory",                                          cmd_mkdir},
    {"touch",     "Create empty file",                                         cmd_touch},
    {"usr",       "User management (add <name> <pass>)",                       cmd_usr},
    {"cat",       "Read file content",                                         cmd_cat},
    {"mdev",      "Device node manager (mdev -s)",                             cmd_mdev},
    {"rm",        "Delete file",                                               cmd_rm},
    {"uptime",    "Shows the current date",                                    cmd_uptime},
    {"bluefetch", "System information",                                        cmd_bluefetch},
    {"logout",    "Close session",                                             cmd_logout},
    {"msg",       "Send a message to the kernel through the connector",          cmd_msg},
    {"halt",      "It stops the system safely",                                cmd_halt},
    {"reboot",    "Restart BlueOS",                                            cmd_reboot},
    {"set",       "Define a variable",                                         cmd_set},
    {"alias",     "Defines or lists command aliases",                          cmd_alias},
    {"losetup",   "Sets and controls loop devices",                            cmd_losetup},
    {"free",   "It shows the amount of free and used memory",                  cmd_free},
    {"env",       "List all variables",                                        cmd_env},
    {"bc",        "Basic Calculator",                                          cmd_bc},
    {"cp",        "Copy files or directories",                                 cmd_cp},
    {"mv",        "Move or rename files/directories",                          cmd_mv},
    {"chmod",     "It allows you to set permissions for directories/files",    cmd_mv},
    {"printf",    "Print formatted text (supports \\n, \\t, %u for user, %t for tty)", cmd_printf},
    {0, 0, 0}
};

/* --- Shell Engine --- */

void print_prompt() {

    if (current_user_index != -1) {
        printk(GREEN, "%s@blueos", users[current_user_index].username);
        printk(WHITE, ":");
        printk(CYAN, "%s", users[current_user_index].cwd);
        printk(WHITE, "$ ");
    } else {
        printk(WHITE, "blueos login: ");
    }
}

int execute_single_with_return(char* input) {
    char expanded[INPUT_BUFFER_SIZE];
    expand_variables(input, expanded);

    char* args = get_args(expanded);
    for (int i = 0; commands_table[i].name != 0; i++) {
        if (strcmp(expanded, commands_table[i].name) == 0) {
            return commands_table[i].function(args); // return 0 o 1
        }
    }
    printk(RED, "Unknown command: %s\n", expanded);
    return 1;
}


void execute_shell_command(char* input) {
    char resolved[INPUT_BUFFER_SIZE];
    resolve_alias(input, resolved);
    if (strlen(input) == 0) {
        print_prompt();
        return;
    }

    if (current_user_index == -1) {
            if (strcmp(input, "login") == 0) {
                char temp_user[32];
                char temp_pass[32];
                int i;
                char c;

                printk(WHITE, "Username: ");
                i = 0;
                while (i < 31) {
                    c = raw_get_char();
                    if (c == '\n') {
                        temp_user[i] = '\0';
                        printk(WHITE, "\n");
                        break;
                    } else if (c == '\b' && i > 0) {
                        i--;
                        printk(WHITE, "\b \b");
                    } else if (c >= 32 && c <= 126) {
                        temp_user[i++] = c;
                        char str[2] = {c, '\0'};
                        printk(WHITE, str);
                    }
                    for(volatile int d=0; d<100000; d++);
                }

                printk(WHITE, "Password: ");
                i = 0;
                while (i < 31) {
                    c = raw_get_char();
                    if (c == '\n') {
                        temp_pass[i] = '\0';
                        printk(WHITE, "\n");
                        break;
                    } else if (c == '\b' && i > 0) {
                        i--;
                    } else if (c >= 32 && c <= 126) {
                        temp_pass[i++] = c;
                        printk(WHITE, "*");
                    }
                    for(volatile int d=0; d<100000; d++);
                }

                if (check_login(temp_user, temp_pass)) {
                    strncpy(current_user, temp_user, 31);
                    current_user[31] = '\0';
                    clear_screen();
                    printk(GREEN, "Welcome to BlueOS, %s!\n", current_user);
                } else {

                    printk(RED, "Login incorrect.\n");
                }
            } else {
                printk(RED, "Please type 'login' to continue.\n");
            }
            
            print_prompt();
            return;
        }


    char* next_ptr = input;
    char* token;

    while ((token = strstr(next_ptr, "&&")) != NULL) {
        *token = '\0';

        int result = execute_single_with_return(next_ptr);

        if (result != 0) {
            print_prompt();
            return;
        }

        next_ptr = token + 2;
        while (*next_ptr == ' ') next_ptr++;
    }

    execute_single_with_return(next_ptr);

    print_prompt();
}
