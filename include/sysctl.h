/*
 * BlueOS / include / sysctl.h
 *
 * Copyright (C) 2024-2026  NopAngel <angelgabrielnieto@outlook.com>
 */

#ifndef _SYSCTL_H
#define _SYSCTL_H

typedef struct {
    char *name;         
    void *value;        
    int type;           
    int writable;      
} sysctl_entry_t;

void sysctl_init();
int sysctl_set(const char *name, const char *new_value);
void sysctl_list();

#endif