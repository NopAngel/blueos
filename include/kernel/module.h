/*
 * BlueOS / include / kernel / module.h
 * * Copyright (C) 2024-2026 NopAngel
 */

#ifndef _MODULE_H
#define _MODULE_H

typedef struct {
    char name[32];
    int (*init)(void);   
    void (*exit)(void);  
    char *description;
    char *version;
} module_t;

#define MODULE_INFO(name_str, init_fn, exit_fn) \
    module_t __this_module = {                  \
        .name = name_str,                       \
        .init = init_fn,                        \
        .exit = exit_fn                         \
    };

#endif