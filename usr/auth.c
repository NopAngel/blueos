#include <auth.h>
#include <lib/string.h>
#include <fs/vfs.h>
#include <lib/string.h>
#include <kernel/io.h>
#include <kernel/printk.h>
#include <kernel/colors.h>

user_t users[MAX_USERS];
int current_user_index = -1;
char current_user[32] = {0};

void auth_init() {
   
}

void add_user(const char *name, const char *pass) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (!users[i].active) {
            strncpy(users[i].username, name, 31);
            strncpy(users[i].password, pass, 31);
            strcpy(users[i].cwd, "/");
            users[i].active = 1;
            
            clear_screen();
            printk(GREEN, "User created!");
            return;
        }
    }
}

int check_login(const char *name, const char *pass) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (users[i].active && 
            strcmp(users[i].username, name) == 0 && 
            strcmp(users[i].password, pass) == 0) {
            current_user_index = i;
            return 1;
        }
    }
    return 0;
}