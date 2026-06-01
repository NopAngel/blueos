#include <stdint.h>
#include <kernel/printk.h>

extern void aes_encrypt_block_rv32(uint32_t state[4], uint32_t* round_keys, int rounds);

typedef struct {
    uint32_t round_keys[60];
    int rounds;
} aes_ctx_t;

void blueos_aes_encrypt_buffer(uint8_t *data, uint32_t len, aes_ctx_t *ctx) {
    if (len % 16 != 0) {
        printk("AES Error: Data length must be multiple of 16\n");
        return;
    }

    for (uint32_t i = 0; i < len; i += 16) {

        uint32_t *state = (uint32_t *)&data[i];

        aes_encrypt_block_rv32(state, ctx->round_keys, ctx->rounds);
    }
}


void crypto_init_test() {
    printk("[CRYPTO] Initializing AES-RISCV Hardware Glue...\n");

    aes_ctx_t my_ctx;
    my_ctx.rounds = 10; // AES-128

    uint8_t test_data[16] = "Hello BlueOS!!!";

    blueos_aes_encrypt_buffer(test_data, 16, &my_ctx);

    printk("[CRYPTO] Test block encrypted successfully!\n");
}
