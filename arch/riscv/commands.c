#include <lib/string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <drivers/keyboard.h>
#include <kernel/printk.h>
#include <kernel/colors.h>
#include <kernel/commands.h>
#include <kernel/io.h>
#include <version.h>
void list_matches(char* prefix);
void add_to_history(char* cmd);
extern void sys_reboot();
extern void sys_shutdown();
extern int current_user_index;
extern char current_user[];

char command_history[HISTORY_MAX][INPUT_BUFFER_SIZE];
int history_count = 0;
int history_current_index;
int history_browse_index = -1;

/* Tablas Globales */
env_var_t env_vars[MAX_ENV_VARS];
alias_t alias_table[MAX_ALIAS];

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


void add_to_history(char* cmd) {
    if (strlen(cmd) == 0) return;
    if (history_count > 0 && strcmp(cmd, command_history[(history_count - 1) % HISTORY_MAX]) == 0) return;

    strncpy(command_history[history_count % HISTORY_MAX], cmd, INPUT_BUFFER_SIZE);
    history_count++;
    history_current_index = -1;
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
            while (*val != '\0') *dest++ = *val++;
        } else {
            *dest++ = *src++;
        }
    }
    *dest = '\0';
}

int cmd_help(char* args) {
    printk(WHITE, "\nBlueOS RISC-V Shell Commands:\n");
    for (int i = 0; commands_table[i].name != 0; i++) {
        printk(CYAN, "  %s ", commands_table[i].name);
        printk(GRAY, "- %s\n", commands_table[i].description);
    }
    return 0;
}

int cmd_echo(char* args) {
    printk(WHITE, "%s\n", args);
    return 0;
}

int cmd_version(char* args) {
    printk(CYAN, "\nBlueOS Kernel v%s\n", UTS_RELEASE);
    printk(WHITE, "Architecture: RISC-V (rv32i_zicsr)\n");
    printk(WHITE, "Compiler: %s\n", COMPILER_INFO);
    return 0;
}

int cmd_clear(char* args) {
    clear_screen();
    return 0;
}

int cmd_reboot(char* args) {
    sys_reboot(); 
    return 0;
}

int cmd_halt(char* args) {
    printk(RED, "\nShutting down BlueOS (RISC-V)...\n");
    sys_shutdown(); 
    return 0;
}

int cmd_set(char* args) {
    char *name = args;
    char *value = strchr(args, '=');
    if (value) {
        *value = '\0';
        value++;
        set_env_var(name, value);
        printk(GREEN, "Env set: %s=%s\n", name, value);
    } else {
        printk(RED, "Usage: set NAME=VALUE\n");
    }
    return 0;
}

int cmd_env(char* args) {
    printk(CYAN, "\nCurrent Environment Variables:\n");
    for (int i = 0; i < MAX_ENV_VARS; i++) {
        if (env_vars[i].active) {
            printk(WHITE, "  %s=%s\n", env_vars[i].name, env_vars[i].value);
        }
    }
    return 0;
}

int cmd_mkdir(char* args)
{
    mkdir(args);
}

int cmd_cd(char* args)
{
    cd(args);
}

int cmd_touch(char* args)
{
    touch(args);
}

int cmd_whoami(char* args) {
    if (current_user_index != -1)
        printk(CYAN, "Current user: %s\n", current_user);
    else
        printk(RED, "Not logged in.\n");
    return 0;
}


int cmd_ls(char* args)
{
    list_items();
}

shell_command_t commands_table[] = {
    {"help",      "Show this help menu",            cmd_help},
    {"echo",      "Print text to terminal",         cmd_echo},
    {"version",   "Show system version",            cmd_version},
    {"clear",     "Clear the screen",               cmd_clear},
    {"whoami",    "Show current user",              cmd_whoami},
    {"set",       "Define an environment variable", cmd_set},
    {"env",       "List all environment variables", cmd_env},
    {"halt",      "Power off the system",           cmd_halt},
    {"reboot",    "Restart the system",             cmd_reboot},
    {"mkdir",     "Restart the system",             cmd_mkdir},
    {"touch",     "Create a file",                  cmd_touch},
    {"cd",        "Create a folder",                cmd_cd},
    {"ls",        "List folders/files",             cmd_ls},
    {0, 0, 0} 
};


void execute_shell_command(char* input) {
    if (strlen(input) == 0) return;

    char expanded[INPUT_BUFFER_SIZE];
    expand_variables(input, expanded);
    
    char* args = get_args(expanded);
    
    for (int i = 0; commands_table[i].name != 0; i++) {
        if (strcmp(expanded, commands_table[i].name) == 0) {
            commands_table[i].function(args);
            return;
        }
    }
    
    printk(RED, "Unknown command: %s\n", expanded);
}