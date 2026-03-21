#include <stdint.h>
#include <stddef.h> 
#include <lib/string.h>  
#include <kernel/printk.h>
#include <kernel/colors.h>


int32_t g_thermal_limit = 80;    
int g_debug_enabled = 0;        
int g_current_color = 0x0F;
typedef enum { CONF_INT, CONF_BOOL } conf_type_t;
typedef struct {
    const char* key;
    void* ptr;
    conf_type_t type;
} kernel_var_t;

kernel_var_t live_vars[] = {
    {"thermal_limit", &g_thermal_limit, CONF_INT},
    {"debug_mode",    &g_debug_enabled, CONF_BOOL},
    {"shell_color",   &g_current_color, CONF_INT},
    {NULL, NULL, 0}
};

int kernel_atoi(const char* s) {
    int res = 0;
    while (*s >= '0' && *s <= '9') {
        res = res * 10 + (*s - '0');
        s++;
    }
    return res;
}

void live_config_apply(char* line) {
    char *equal_sign = strchr(line, '=');
    if (!equal_sign) {
        printk(RED, "[JIT] ERROR: Incorrect format. United States var=value\n");
        return;
    }


    *equal_sign = '\0';
    char* key = line;
    char* value = equal_sign + 1;

    if (*value == ' ') value++;

    for(int i = 0; live_vars[i].key != NULL; i++) {
        if (strcmp(key, live_vars[i].key) == 0) {
            if (live_vars[i].type == CONF_INT || live_vars[i].type == CONF_BOOL) {
                int val = kernel_atoi(value);
                *((int*)live_vars[i].ptr) = val;
                printk(GREEN, "[JIT] %s update in %d\n", key, val);
            }
            return;
        }
    }
    printk(RED, "[JIT] Variable not found: %s\n", key);
}