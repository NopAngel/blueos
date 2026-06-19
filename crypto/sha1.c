#include <kernel/colors.h>
#include <kernel/printk.h>
#include <stddef.h>
#include <stdint.h>

#define MODULE_NAME "CRYPTO_SHA1"

#define SHA1_ROL(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

typedef struct {
  uint32_t state[5];
  uint64_t count;
  uint8_t buffer[64];
} crypto_sha1_ctx_t;

/**
 * crypto_sha1_transform: Core processing block operation.
 */
static void crypto_sha1_transform(uint32_t state[5], const uint8_t buffer[64]) {
  uint32_t a = state[0], b = state[1], c = state[2], d = state[3], e = state[4];
  uint32_t w[80];

  /* Explode 64 bytes input into 16 32-bit words */
  for (int i = 0; i < 16; i++) {
    w[i] = ((uint32_t)buffer[i * 4] << 24) |
           ((uint32_t)buffer[i * 4 + 1] << 16) |
           ((uint32_t)buffer[i * 4 + 2] << 8) | ((uint32_t)buffer[i * 4 + 3]);
  }

  /* Extend into 80 words tracking array space */
  for (int i = 16; i < 80; i++) {
    w[i] = SHA1_ROL(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
  }

  /* Round 1 (0 to 19 iterations) */
  for (int i = 0; i < 20; i++) {
    uint32_t temp =
        SHA1_ROL(a, 5) + ((b & c) | (~b & d)) + e + w[i] + 0x5A827999;
    e = d;
    d = c;
    c = SHA1_ROL(b, 30);
    b = a;
    a = temp;
  }

  /* Round 2 (20 to 39 iterations) */
  for (int i = 20; i < 40; i++) {
    uint32_t temp = SHA1_ROL(a, 5) + (b ^ c ^ d) + e + w[i] + 0x6ED9EBA1;
    e = d;
    d = c;
    c = SHA1_ROL(b, 30);
    b = a;
    a = temp;
  }

  /* Round 3 (40 to 59 iterations) */
  for (int i = 40; i < 60; i++) {
    uint32_t temp =
        SHA1_ROL(a, 5) + ((b & c) | (b & d) | (c & d)) + e + w[i] + 0x8F1BBCDC;
    e = d;
    d = c;
    c = SHA1_ROL(b, 30);
    b = a;
    a = temp;
  }

  /* Round 4 (60 to 79 iterations) */
  for (int i = 60; i < 80; i++) {
    uint32_t temp = SHA1_ROL(a, 5) + (b ^ c ^ d) + e + w[i] + 0xCA62C1D6;
    e = d;
    d = c;
    c = SHA1_ROL(b, 30);
    b = a;
    a = temp;
  }

  /* Add working values to hash state values */
  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;
  state[4] += e;
}

/**
 * crypto_sha1_init: Setup default initial vectors tracking handles.
 */
void crypto_sha1_init(crypto_sha1_ctx_t *ctx) {
  ctx->count = 0;
  ctx->state[0] = 0x67452301;
  ctx->state[1] = 0xEFCDAB89;
  ctx->state[2] = 0x98BADCFE;
  ctx->state[3] = 0x10325476;
  ctx->state[4] = 0xC3D2E1F0;

  printk("<6>[  %s  ] Secure Hashing Context structure loaded.\n", MODULE_NAME);
}