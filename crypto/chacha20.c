#include <kernel/colors.h>
#include <kernel/printk.h>
#include <stddef.h>
#include <stdint.h>

#define MODULE_NAME "CRYPTO_CHACHA20"

#define ROTL32(v, n) (((v) << (n)) | ((v) >> (32 - (n))))

/* ChaCha20 Quarter Round Core Macro */
#define CHACHA20_QUARTERROUND(a, b, c, d)                                      \
  a += b;                                                                      \
  d ^= a;                                                                      \
  d = ROTL32(d, 16);                                                           \
  c += d;                                                                      \
  b ^= c;                                                                      \
  b = ROTL32(b, 12);                                                           \
  a += b;                                                                      \
  d ^= a;                                                                      \
  d = ROTL32(d, 8);                                                            \
  c += d;                                                                      \
  b ^= c;                                                                      \
  b = ROTL32(b, 7);

typedef struct {
  uint32_t state[16];
} chacha20_ctx_t;

/**
 * crypto_chacha20_init: Sets up the 512-bit initial state matrix using the
 * classic "expand 32-byte k" constants, a 256-bit key, and a 64-bit nonce.
 */
void crypto_chacha20_init(chacha20_ctx_t *ctx, const uint8_t key[32],
                          const uint8_t nonce[8], uint64_t counter) {
  /* Magic Constants: "expand 32-byte k" */
  ctx->state[0] = 0x61786520;
  ctx->state[1] = 0x3320646e;
  ctx->state[2] = 0x79622d32;
  ctx->state[3] = 0x6b206574;

  /* Unpack 32-byte key into states 4 through 11 */
  for (int i = 0; i < 8; i++) {
    ctx->state[4 + i] =
        ((uint32_t)key[i * 4]) | ((uint32_t)key[i * 4 + 1] << 8) |
        ((uint32_t)key[i * 4 + 2] << 16) | ((uint32_t)key[i * 4 + 3] << 24);
  }

  /* Block Counter setup */
  ctx->state[12] = (uint32_t)(counter & 0xFFFFFFFF);
  ctx->state[13] = (uint32_t)(counter >> 32);

  /* Unpack 8-byte Nonce into states 14 and 15 */
  ctx->state[14] = ((uint32_t)nonce[0]) | ((uint32_t)nonce[1] << 8) |
                   ((uint32_t)nonce[2] << 16) | ((uint32_t)nonce[3] << 24);
  ctx->state[15] = ((uint32_t)nonce[4]) | ((uint32_t)nonce[5] << 8) |
                   ((uint32_t)nonce[6] << 16) | ((uint32_t)nonce[7] << 24);

  printk("<6>[  %s  ] Core matrix state initialized successfully.\n",
         MODULE_NAME);
}

/**
 * crypto_chacha20_block: Computes a single 64-byte keystream block execution
 * frame.
 */
void crypto_chacha20_block(chacha20_ctx_t *ctx, uint8_t output[64]) {
  uint32_t x[16];
  for (int i = 0; i < 16; i++)
    x[i] = ctx->state[i];

  /* 20 Rounds: 10 iterations of Column and Diagonal rounds */
  for (int i = 0; i < 10; i++) {
    /* Column Rounds */
    CHACHA20_QUARTERROUND(x[0], x[4], x[8], x[12]);
    CHACHA20_QUARTERROUND(x[1], x[5], x[9], x[13]);
    CHACHA20_QUARTERROUND(x[2], x[6], x[10], x[14]);
    CHACHA20_QUARTERROUND(x[3], x[7], x[11], x[15]);
    /* Diagonal Rounds */
    CHACHA20_QUARTERROUND(x[0], x[5], x[10], x[15]);
    CHACHA20_QUARTERROUND(x[1], x[6], x[11], x[12]);
    CHACHA20_QUARTERROUND(x[2], x[7], x[8], x[13]);
    CHACHA20_QUARTERROUND(x[3], x[4], x[9], x[14]);
  }

  /* Add initial state back to final block and pack into byte array */
  for (int i = 0; i < 16; i++) {
    uint32_t val = x[i] + ctx->state[i];
    output[i * 4] = (uint8_t)(val & 0xFF);
    output[i * 4 + 1] = (uint8_t)((val >> 8) & 0xFF);
    output[i * 4 + 2] = (uint8_t)((val >> 16) & 0xFF);
    output[i * 4 + 3] = (uint8_t)((val >> 24) & 0xFF);
  }

  ctx->state[12]++; /* Increment block counter offset */
}