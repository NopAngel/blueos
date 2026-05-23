#include <arch/riscv/aes_macros.h>
#include <stdint.h>

#define ENABLE_AES_EXT  asm volatile (".option push\n\t.option arch, +zkn")
#define DISABLE_AES_EXT asm volatile (".option pop")

static inline uint32_t aes32_encrypt_step(uint32_t rs1, uint32_t rs2, int bs) {
    uint32_t rt;
    ENABLE_AES_EXT;
    
    if (bs == 0)      asm volatile ("aes32esi %0, %1, %2, 0" : "=r"(rt) : "r"(rs1), "r"(rs2));
    else if (bs == 1) asm volatile ("aes32esi %0, %1, %2, 1" : "=r"(rt) : "r"(rs1), "r"(rs2));
    else if (bs == 2) asm volatile ("aes32esi %0, %1, %2, 2" : "=r"(rt) : "r"(rs1), "r"(rs2));
    else              asm volatile ("aes32esi %0, %1, %2, 3" : "=r"(rt) : "r"(rs1), "r"(rs2));


    DISABLE_AES_EXT;
    return rt;
}


void aes_encrypt_block_rv32(uint32_t state[4], uint32_t* round_keys, int rounds) {
    // 1. AddRoundKey init
    for (int i = 0; i < 4; i++) state[i] ^= round_keys[i];

    for (int r = 1; r < rounds; r++) {
        uint32_t next_state[4] = {0};
        uint32_t* rk = &round_keys[r * 4];

        for (int i = 0; i < 4; i++) {
            next_state[i] ^= aes32_encrypt_step(state[i], state[(i+1)%4], 0);
            next_state[i] ^= rk[i];
        }
        
        for (int i = 0; i < 4; i++) state[i] = next_state[i];
    }

}