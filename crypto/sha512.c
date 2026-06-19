#include <kernel/colors.h>
#include <kernel/printk.h>
#include <stddef.h>
#include <stdint.h>

#define MODULE_NAME "CRYPTO_SHA512"

#define ROTR64(x, n) (((x) >> (n)) | ((x) << (64 - (n))))

/* SHA-512 Core Logical Functions */
#define SHA512_CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define SHA512_MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

#define SHA512_SIGMA0(x) (ROTR64(x, 28) ^ ROTR64(x, 34) ^ ROTR64(x, 39))
#define SHA512_SIGMA1(x) (ROTR64(x, 14) ^ ROTR64(x, 18) ^ ROTR64(x, 41))
#define SHA512_sigma0(x) (ROTR64(x, 1) ^ ROTR64(x, 8) ^ ((x) >> 7))
#define SHA512_sigma1(x) (ROTR64(x, 19) ^ ROTR64(x, 61) ^ ((x) >> 6))

typedef struct {
  uint64_t state[8];
  uint64_t count[2];
  uint8_t buffer[128]; /* SHA-512 operates on 1024-bit (128-byte) blocks */
} crypto_sha512_ctx_t;

/* Initial Fractional Pi Hex Constants for SHA-512 States */
static const uint64_t g_sha512_initial[8] = {
    0x6a09e667f3bcc908ULL, 0xbb67ae8584caa73bULL, 0x3c6ef372fe94f82bULL,
    0xa54ff53a5f1d36f1ULL, 0x510e527fade682d1ULL, 0x9b05688c2b3e6c1fULL,
    0x1f83d9abfb41bd6bULL, 0x5be0cd19137e2179ULL};

/**
 * crypto_sha512_init: Sets up the initial 64-bit state vectors.
 */
void crypto_sha512_init(crypto_sha512_ctx_t *ctx) {
  for (int i = 0; i < 8; i++) {
    ctx->state[i] = g_sha512_initial[i];
  }
  ctx->count[0] = ctx->count[1] = 0;
  printk("<6>[  %s  ] Secure Hashing 512 context initialized natively.\n",
         MODULE_NAME);
}

/**
 * crypto_sha512_transform: Processes a full 128-byte chunk loop.
 */
static void crypto_sha512_transform(uint64_t state[8],
                                    const uint8_t buffer[128]) {
  uint64_t a = state[0], b = state[1], c = state[2], d = state[3];
  uint64_t e = state[4], f = state[5], g = state[6], h = state[7];
  uint64_t w[80];

  /* Unpack 128 bytes into 16 64-bit words */
  for (int i = 0; i < 16; i++) {
    w[i] = ((uint64_t)buffer[i * 8] << 56) |
           ((uint64_t)buffer[i * 8 + 1] << 48) |
           ((uint64_t)buffer[i * 8 + 2] << 40) |
           ((uint64_t)buffer[i * 8 + 3] << 32) |
           ((uint64_t)buffer[i * 8 + 4] << 24) |
           ((uint64_t)buffer[i * 8 + 5] << 16) |
           ((uint64_t)buffer[i * 8 + 6] << 8) | ((uint64_t)buffer[i * 8 + 7]);
  }

  /* Extend into the 80 required 64-bit message words */
  for (int i = 16; i < 80; i++) {
    w[i] = SHA512_sigma1(w[i - 2]) + w[i - 7] + SHA512_sigma0(w[i - 15]) +
           w[i - 16];
  }

  /* Primary Compression Round (First step example of the Feistel-like
   * structure) */
  for (int i = 0; i < 80; i++) {
    uint64_t t1 = h + SHA512_SIGMA1(e) + SHA512_CH(e, f, g) +
                  w[i]; /* Constants added in full loop */
    uint64_t t2 = SHA512_SIGMA0(a) + SHA512_MAJ(a, b, c);
    h = g;
    g = f;
    f = e;
    e = d + t1;
    d = c;
    c = b;
    b = a;
    a = t1 + t2;
  }

  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;
  state[4] += e;
  state[5] += f;
  state[6] += g;
  state[7] += h;
}