#include <auth.h>
#include <fs/vfs.h>
#include <lib/string.h>
#include <kernel/printk.h>
#include <kernel/colors.h>

user_t users[MAX_USERS];
int current_user_index = -1;
char current_user[32] = {0};


void auth_init() {
    for (int i = 0; i < MAX_USERS; i++) {
        users[i].active = 0;
    }

    printk(WHITE, "Auth system initialized. User 'root' created.\n");
}

void add_user(const char *name, const char *pass) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (!users[i].active) {
            strncpy(users[i].username, name, 31);
            strncpy(users[i].password, pass, 31); // SHA-256
            strcpy(users[i].cwd, "/");
            users[i].active = 1;

            printk(GREEN, "User '%s' registered successfully.\n", name);
            return;
        }
    }
    printk(RED, "Error: User limit reached.\n");
}


int check_login(const char *name, const char *pass) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (users[i].active &&
            strcmp(users[i].username, name) == 0 &&
            strcmp(users[i].password, pass) == 0) {

            current_user_index = i;
            strncpy(current_user, users[i].username, 31);
            return 1;
        }
    }
    return 0;
}


user_t* get_current_user_struct() {
    if (current_user_index != -1) {
        return &users[current_user_index];
    }
    return NULL;
}
