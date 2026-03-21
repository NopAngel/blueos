#ifndef JUMP_LABEL_H
#define JUMP_LABEL_H

#include <stdint.h>

struct jump_entry {
    uint32_t code;   
    uint32_t target;  
    uint32_t key;    
};

void static_key_update(uint32_t *key, int enable);

#define STATIC_KEY_FALSE(name) \
    ({ \
        asm volatile goto ( \
            "1: .option push          \n\t" \
            "   .option arch, +c      \n\t" \
            "   nop                   \n\t" \
            "   .option pop           \n\t" \
            "   .section __jump_table, \"a\" \n\t" \
            "   .align 2              \n\t" \
            "   .word 1b, %l[l_yes], %0 \n\t" \
            "   .previous             \n\t" \
            : : "i" (&name) : : l_yes); \
        0; \
    l_yes: \
        1; \
    })

#endif