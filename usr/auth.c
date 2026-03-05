#include <auth.h>
#include <lib/string.h>
#include <fs/vfs.h>
#include <lib/string.h>
#include <blueos/io.h>
#include <blueos/printk.h>
#include <blueos/colors.h>

user_t users[MAX_USERS];
int current_user_index = -1;
char current_user[32] = {0};

void save_users_to_fs() {
    vfs_write("users.dat", (const char*)users);
}


void load_users_from_fs() {
    char* data = vfs_read("users.dat");
    
    if (data != NULL) {
        memcpy(users, data, sizeof(user_t) * MAX_USERS);
        printk(GREEN, "[  OK  ] ");
        printk(WHITE, "Auth: User database loaded from VFS.\n");
    } else {
        for (int i = 0; i < MAX_USERS; i++) {
            users[i].active = 0;
            strcpy(users[i].cwd, "/");
        }
        add_user("admin", "1234");
        printk(YELLOW, "[ INFO ] ");
        printk(WHITE, "Auth: No database found. Admin created.\n");
    }
}

void auth_init() {
    load_users_from_fs();
}

void add_user(const char *name, const char *pass) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (!users[i].active) {
            strncpy(users[i].username, name, 31);
            strncpy(users[i].password, pass, 31);
            strcpy(users[i].cwd, "/");
            users[i].active = 1;
            
            save_users_to_fs(); 
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