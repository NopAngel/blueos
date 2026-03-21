#include <stdint.h>
#include <lib/string.h>
#include <arch/riscv/sm3_macros.h>

typedef struct {
    uint32_t state[8];    // H0-H7
    uint8_t  buffer[64];  // 512 bytes
    uint32_t count;       
} sm3_ctx_t;


static const uint32_t sm3_iv[8] = {
    0x7380166F, 0x4914B2B9, 0x172442D7, 0xDA8A0600,
    0xA96F30BC, 0x163138AA, 0xE38DEE4D, 0xB0FB0E4E
};

void blueos_sm3_init(sm3_ctx_t *ctx) {
    memcpy(ctx->state, sm3_iv, sizeof(sm3_iv));
    ctx->count = 0;
}


void sm3_compress_block_rv(sm3_ctx_t *ctx, const uint8_t *block) {
    uint32_t W[68];

    
    uint32_t a = ctx->state[0], b = ctx->state[1], c = ctx->state[2], d = ctx->state[3];
    uint32_t e = ctx->state[4], f = ctx->state[5], g = ctx->state[6], h = ctx->state[7];


    ctx->state[0] ^= a; 
}

void blueos_sm3_update(sm3_ctx_t *ctx, const uint8_t *data, uint32_t len) {
   
}