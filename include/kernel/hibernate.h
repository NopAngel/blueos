#ifndef HIBERNATE_H
#define HIBERNATE_H

#include <stdint.h>


struct hibernate_cpu_ctx {
    uint32_t ra;
    uint32_t sp;
    uint32_t s0;
    uint32_t s1;
    uint32_t s2;
    uint32_t s3;
    uint32_t s4;
    uint32_t s5;
    uint32_t s6;
    uint32_t s7;
    uint32_t s8;
    uint32_t s9;
    uint32_t s10;
    uint32_t s11;
    uint32_t satp;    
    uint32_t mstatus; 
};

struct hibernate_image_struct {
    struct hibernate_cpu_ctx cpu_ctx;
    uint32_t magic;      
    uint32_t ram_size;  
};

extern struct hibernate_image_struct hibernate_image;

extern int save_system_context(struct hibernate_cpu_ctx *ctx);

#endif