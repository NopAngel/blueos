/*
 * BlueOS / kernel / ksyms.c
 *
 * Copyright (C) 2024-2026  NopAngel <angelgabrielnieto@outlook.com>
 */

#include <include/printk.h>    
#include <include/sysctl.h>   

typedef struct {
    char *name;
    void *address;
    unsigned int crc; 
} kernel_symbol_t;


kernel_symbol_t ksym_table[] = {
    {"printk", (void*)&printk, 0xABC123},
    {"sysctl_set", (void*)&sysctl_set, 0xDEF456},
    {0, 0, 0} 
};