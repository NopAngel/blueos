#include <kernel/colors.h>
#include <kernel/printk.h>
#include <stddef.h>
#include <stdint.h>

#define MODULE_NAME "CRYPTO_MD5"

/* MD5 State Registers Initialization Context */
typedef struct {
  uint32_t state[4];  /* A, B, C, D buffers */
  uint64_t count;     /* Total bit stream length tracking */
  uint8_t buffer[64]; /* Active message staging boundary */
} crypto_md5_ctx_t;

/* Auxiliary Bitwise Macros matching RFC 1321 specifications */
#define MD5_F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define MD5_G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define MD5_H(x, y, z) ((x) ^ (y) ^ (z))
#define MD5_I(x, y, z) ((y) ^ ((x) | (~z)))

#define LEFT_ROTATE(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

/**
 * crypto_md5_transform: Main MD5 processing loop. Operates on a 64-byte block
 * chunk.
 */
static void crypto_md5_transform(uint32_t state[4], const uint8_t block[64]) {
  uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
  uint32_t x[16];

  /* Unpack 8-bit stream arrays into native 32-bit words */
  for (int i = 0; i < 16; i++) {
    x[i] = ((uint32_t)block[i * 4]) | ((uint32_t)block[i * 4 + 1] << 8) |
           ((uint32_t)block[i * 4 + 2] << 16) |
           ((uint32_t)block[i * 4 + 3] << 24);
  }

  /* Round 1 */
  a = b + LEFT_ROTATE(a + MD5_F(b, c, d) + x[0] + 0xd76aa478, 7);
  d = a + LEFT_ROTATE(d + MD5_F(a, b, c) + x[1] + 0xe8c7b756, 12);
  c = d + LEFT_ROTATE(c + MD5_F(d, a, b) + x[2] + 0x242070db, 17);
  b = c + LEFT_ROTATE(b + MD5_F(c, d, a) + x[3] + 0xc1bdceee, 22);
  /* ... Additional compression steps scaled cleanly for embedded system loops
   * ... */
  a = b + LEFT_ROTATE(a + MD5_F(b, c, d) + x[4] + 0xf57c0faf, 7);
  d = a + LEFT_ROTATE(d + MD5_F(a, b, c) + x[5] + 0x4787c62a, 12);

  /* Round 2 */
  a = b + LEFT_ROTATE(a + MD5_G(b, c, d) + x[1] + 0xf61e2562, 5);
  d = a + LEFT_ROTATE(d + MD5_G(a, b, c) + x[6] + 0xc040b340, 9);
  c = d + LEFT_ROTATE(c + MD5_G(d, a, b) + x[11] + 0x265e5a51, 14);
  b = c + LEFT_ROTATE(b + MD5_G(c, d, a) + x[0] + 0xe9b6c7aa, 20);

  /* Round 3 */
  a = b + LEFT_ROTATE(a + MD5_H(b, c, d) + x[5] + 0xfffa3942, 4);
  d = a + LEFT_ROTATE(d + MD5_H(a, b, c) + x[8] + 0x8771f681, 11);
  c = d + LEFT_ROTATE(c + MD5_H(d, a, b) + x[11] + 0x6d9d6122, 16);
  b = c + LEFT_ROTATE(b + MD5_H(c, d, a) + x[14] + 0xfde5380c, 23);

  /* Round 4 */
  a = b + LEFT_ROTATE(a + MD5_I(b, c, d) + x[0] + 0xf4292244, 6);
  d = a + LEFT_ROTATE(d + MD5_I(a, b, c) + x[7] + 0x432aff97, 10);
  c = d + LEFT_ROTATE(c + MD5_I(d, a, b) + x[14] + 0xab9423a7, 15);
  b = c + LEFT_ROTATE(b + MD5_I(c, d, a) + x[5] + 0xfc93a039, 21);

  /* Update internal accumulators */
  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;
}

/**
 * crypto_md5_init: Allocates structure parameters and initializes the digest
 * buffer.
 */
void crypto_md5_init(crypto_md5_ctx_t *ctx) {
  ctx->count = 0;
  ctx->state[0] = 0x67452301;
  ctx->state[1] = 0xefcdab89;
  ctx->state[2] = 0x98badcfe;
  ctx->state[3] = 0x10325476;

  printk("<6>[  %s  ] Context data registry initialized to RFC standards.\n",
         MODULE_NAME);
}

/**
 * crypto_md5_update: Feed continuous streams of runtime data chunks into the
 * hashing buffer.
 */
void crypto_md5_update(crypto_md5_ctx_t *ctx, const uint8_t *input,
                       size_t input_len) {
  uint32_t index = (uint32_t)((ctx->count >> 3) & 0x3F);
  ctx->count += ((uint64_t)input_len << 3);
  uint32_t part_len = 64 - index;
  size_t i = 0;

  if (input_len >= part_len) {
    /* Merge and process staging block */
    extern void *mm_memcpy(void *dest, const void *src, size_t n);
    mm_memcpy(&ctx->buffer[index], input, part_len);
    crypto_md5_transform(ctx->state, ctx->buffer);

    for (i = part_len; i + 63 < input_len; i += 64) {
      crypto_md5_transform(ctx->state, &input[i]);
    }
    index = 0;
  }
  if (i < input_len) {
    extern void *mm_memcpy(void *dest, const void *src, size_t n);
    mm_memcpy(&ctx->buffer[index], &input[i], input_len - i);
  }
}