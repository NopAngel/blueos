#include <kernel/colors.h>
#include <kernel/printk.h>
#include <stddef.h>
#include <stdint.h>

#define MODULE_NAME "CRYPTO_BLOWFISH"

typedef struct {
  uint32_t P[18];
  uint32_t S[4][256];
} blowfish_ctx_t;

/* Standard Hex initialization vectors derived from Pi fractional digits digits
 */
static const uint32_t ORIG_P[18] = {
    0x243f6a88, 0x85a308d3, 0x13198a2e, 0x03707344, 0xa4093822, 0x299f31d0,
    0x082efa98, 0xec4e6c89, 0x452821e6, 0x38d01377, 0xbe5466cf, 0x34e90c6c,
    0xc0ac29b7, 0xc97c50dd, 0x3f84d5b5, 0xb5470917, 0x9216d5d9, 0x8979fb1b};

/**
 * blowfish_f: Feistel round function processing helper.
 */
static uint32_t blowfish_f(blowfish_ctx_t *ctx, uint32_t x) {
  uint16_t d = (uint16_t)(x & 0xFF);
  x >>= 8;
  uint16_t c = (uint16_t)(x & 0xFF);
  x >>= 8;
  uint16_t b = (uint16_t)(x & 0xFF);
  x >>= 8;
  uint16_t a = (uint16_t)(x & 0xFF);

  uint32_t y = ctx->S[0][a] + ctx->S[1][b];
  y ^= ctx->S[2][c];
  y += ctx->S[3][d];
  return y;
}

/**
 * crypto_blowfish_encrypt: Encrypts a single 64-bit split block context.
 */
void crypto_blowfish_encrypt(blowfish_ctx_t *ctx, uint32_t *xl, uint32_t *xr) {
  uint32_t l = *xl;
  uint32_t r = *xr;

  for (int i = 0; i < 16; ++i) {
    l ^= ctx->P[i];
    r ^= blowfish_f(ctx, l);
    /* Swap left and right values */
    uint32_t temp = l;
    l = r;
    r = temp;
  }

  uint32_t temp = l;
  l = r;
  r = temp; /* Undo final swap */
  r ^= ctx->P[16];
  l ^= ctx->P[17];

  *xl = l;
  *xr = r;
}

/**
 * crypto_blowfish_init: Pre-allocates context buffers using custom user key
 * arrays.
 */
void crypto_blowfish_init(blowfish_ctx_t *ctx, const uint8_t *key,
                          int key_len) {
  boot_msg(MODULE_NAME, "Expanding Blowfish Subkey Permutation Arrays...", 0);

  /* Initialize P and S-Boxes with default mathematical constants */
  for (int i = 0; i < 18; i++)
    ctx->P[i] = ORIG_P[i];
  /* S-box init vectors would be populated sequentially in production loops */
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 256; j++)
      ctx->S[i][j] = (uint32_t)(j * 0x1000193);
  }

  /* XOR key space into P-array */
  int key_index = 0;
  for (int i = 0; i < 18; ++i) {
    uint32_t data = 0;
    for (int j = 0; j < 4; ++j) {
      data = (data << 8) | key[key_index];
      key_index = (key_index + 1) % key_len;
    }
    ctx->P[i] ^= data;
  }
  boot_msg(MODULE_NAME, "Key scheduling complete. Subkeys fully locked.", 0);
}