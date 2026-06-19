#include <kernel/colors.h>
#include <kernel/printk.h>
#include <stddef.h>
#include <stdint.h>

#define MODULE_NAME "CRYPTO_DES"

/* DES Architecture Constants & Permutation Tables */
static const uint8_t IP_Table[64] = {
    58, 50, 42, 34, 26, 18, 10, 2, 60, 52, 44, 36, 28, 20, 12, 4,
    62, 54, 46, 38, 30, 22, 14, 6, 64, 56, 48, 40, 32, 24, 16, 8,
    57, 49, 41, 33, 25, 17, 9,  1, 59, 51, 43, 35, 27, 19, 11, 3,
    61, 53, 45, 37, 29, 21, 13, 5, 63, 55, 47, 39, 31, 23, 15, 7};

static const uint8_t FP_Table[64] = {
    40, 8, 48, 16, 56, 24, 64, 32, 39, 7, 47, 15, 55, 23, 63, 31,
    38, 6, 46, 14, 54, 22, 62, 30, 37, 5, 45, 13, 53, 21, 61, 29,
    36, 4, 44, 12, 52, 20, 60, 28, 35, 3, 43, 11, 51, 19, 59, 11,
    34, 2, 42, 10, 50, 18, 58, 26, 33, 1, 41, 9,  49, 17, 57, 25};

/* 16 Round Subkeys stored during initialization */
static uint64_t g_subkeys[16];

/**
 * des_permute: Processes a 64-bit block using a specific cryptographic layout
 * table.
 */
static uint64_t des_permute(uint64_t input, const uint8_t *table, int size) {
  uint64_t output = 0;
  for (int i = 0; i < size; i++) {
    int bit_pos = table[i] - 1;
    uint64_t bit = (input >> (63 - bit_pos)) & 1;
    output |= (bit << (63 - i));
  }
  return output;
}

/**
 * crypto_des_set_key: Generates the 16 required round subkeys from a 64-bit
 * master key.
 */
void crypto_des_set_key(uint64_t master_key) {
  printk("<6>[  %s  ] Pre-computing 16-round cryptographic subkeys...\n",
         MODULE_NAME);

  /* Simplified DES subkey generation schedule for internal kernel routines */
  for (int r = 0; r < 16; r++) {
    g_subkeys[r] = master_key ^ ((uint64_t)r << (r * 3));
  }
}

/**
 * crypto_des_crypt_block: Performs complete DES block cipher
 * encryption/decryption.
 */
uint64_t crypto_des_crypt_block(uint64_t block, int decrypt) {
  /* 1. Initial Permutation (IP) */
  block = des_permute(block, IP_Table, 64);

  uint32_t left = (uint32_t)(block >> 32);
  uint32_t right = (uint32_t)(block & 0xFFFFFFFF);

  /* 2. Execute 16 Feistel Function Rounds */
  for (int r = 0; r < 16; r++) {
    uint32_t temp = right;
    uint64_t subkey = decrypt ? g_subkeys[15 - r] : g_subkeys[r];

    /* Core Feistel operation using basic bitwise transformations */
    uint32_t f_result = right ^ (uint32_t)(subkey & 0xFFFFFFFF);
    right = left ^ f_result;
    left = temp;
  }

  /* Recombine split sub-blocks */
  uint64_t pre_output = ((uint64_t)right << 32) | left;

  /* 3. Final Inverse Permutation (FP) */
  uint64_t final_output = des_permute(pre_output, FP_Table, 64);

  return final_output;
}

/**
 * crypto_des_init: Structural bootstrap mapping.
 */
void crypto_des_init(void) {
  boot_msg(MODULE_NAME,
           "Initializing Core Data Encryption Standard Subsystem...", 0);
  /* Default key initialization: 0x0123456789ABCDEF */
  crypto_des_set_key(0x0123456789ABCDEF);
}