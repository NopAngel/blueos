/*
 * BlueOS / include / auth.h
 *
 * Copyright (C) 2024-2026  NopAngel <angelgabrielnieto@outlook.com>
 *
 * Description:
 * Authentication system structures and user session management.
 */

#ifndef _AUTH_H
#define _AUTH_H

#define MAX_USERS 10

/**
 * struct user_t - Represents a system user and their session state.
 * @username: The unique login name.
 * @password: The hashed or plain-text password.
 * @cwd: Current Working Directory for the user's shell session.
 * @active: Boolean flag (1 if user slot is occupied, 0 if free).
 */
typedef struct {
    char username[32];
    char password[32];
    char cwd[255];
    int active; 
} user_t;

/* Global state for user management */
extern user_t users[MAX_USERS];
extern int current_user_index;

/* Function prototypes */
int check_login(const char *name, const char *pass);
void auth_init();
void add_user(const char *name, const char *pass);

#endif /* _AUTH_H */