/*
 * BlueOS / usr / auth.c
 *
 * Copyright (C) 2024-2026  NopAngel <angelgabrielnieto@outlook.com>
 *
 * Description:
 * User authentication and session control implementation.
 */

#include <include/auth.h>
#include <include/lib/string.h>

user_t users[MAX_USERS];
int current_user_index = {};

char current_user[32] = {0};

/**
 * auth_init() - Wipe user table and set default security state.
 */
void auth_init() {
    for (int i = 0; i < MAX_USERS; i++) {
        users[i].active = 0;
        strcpy(users[i].cwd, "/"); // Default directory
    }
    
    /* Create a default root user for development */
    add_user("admin", "1234");
}

/**
 * add_user() - Register a new user in the first available slot.
 * @name: Username string.
 * @pass: Password string.
 */
void add_user(const char *name, const char *pass) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (!users[i].active) {
            strcpy(users[i].username, name);
            strcpy(users[i].password, pass);
            strcpy(users[i].cwd, "/");
            users[i].active = 1;
            return;
        }
    }
}

/**
 * check_login() - Validate credentials and open a session.
 * @name: Input username.
 * @pass: Input password.
 * Return: 1 on success, 0 on failure.
 */
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