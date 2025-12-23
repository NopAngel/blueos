/*aes-neon.s: Asm*/
/*
*
* License: GPL-3.0
*
*/

.text
.arch armv8-a
.cpu cortex-a72

.align 4
Te0:
.word 0xc66363a5,0xf87c7c84,0xee777799,0xf67b7b8d
.word 0xfff2f20d,0xd66b6bbd,0xde6f6fb1,0x91c5c554
.word 0x60303050,0x02010103,0xce6767a9,0x562b2b7d
.word 0xe7fefe19,0xb5d7d762,0x4dababe6,0xec76769a

.align 4
Te1:
.word 0x00000000,0x0b0d090e,0x161a121c,0x1d171b12
.word 0x2c342438,0x27392d36,0x3a2e3624,0x31233f2a

.align 4
Te2:
.word 0x00000000,0x0d090b0e,0x1a12161c,0x171b1d12
.word 0x34242c38,0x392d2736,0x2e363a24,0x233f312a

.align 4
Te3:
.word 0x00000000,0x090e0b0d,0x121c161a,0x1b121d17
.word 0x24382c34,0x2d362739,0x36243a2e,0x3f2a3123

.align 4
Td0:
.word 0x51f4a750,0x7e416553,0x1a17a4c3,0x3a275e96
.word 0x3bab6bcb,0x1f9d45f1,0xacfa58ab,0x4be30393
.word 0x2030fa55,0xad766df6,0x88cc7691,0xf5024c25
.word 0x4fe5d7fc,0xc52acbd7,0x26354480,0xb562a38f

.align 4
Td1:
.word 0x00000000,0x0d0b0e09,0x1a161c12,0x17121d1b
.word 0x3424382c,0x392d3627,0x2e36243a,0x233f2a31

.align 4
Td2:
.word 0x00000000,0x0b0e090d,0x161c121a,0x121d1b17
.word 0x24382c34,0x2d362739,0x36243a2e,0x3f2a3123

.align 4
Td3:
.word 0x00000000,0x0e090d0b,0x1c121a16,0x1d1b1712
.word 0x382c3424,0x2d27392d,0x243a2e36,0x2a31233f

.align 4
Rcon:
.word 0x01000000,0x02000000,0x04000000,0x08000000
.word 0x10000000,0x20000000,0x40000000,0x80000000
.word 0x1B000000,0x36000000

.global aes128_encrypt_block_neon
.global aes128_decrypt_block_neon
.global aes128_key_expand_neon
.global aes256_encrypt_block_neon
.global aes256_decrypt_block_neon
.global aes256_key_expand_neon

aes128_encrypt_block_neon:
    ld1 {v0.16b}, [x0]
    ld1 {v1.16b}, [x2], #16
    eor v0.16b, v0.16b, v1.16b
    mov w3, #9
1:
    ld1 {v16.16b-v19.16b}, [x1], #64
    tbl v2.16b, {v0.16b}, v16.16b
    tbl v3.16b, {v0.16b}, v17.16b
    tbl v4.16b, {v0.16b}, v18.16b
    tbl v5.16b, {v0.16b}, v19.16b
    eor v2.16b, v2.16b, v3.16b
    eor v4.16b, v4.16b, v5.16b
    eor v0.16b, v2.16b, v4.16b
    shl v2.16b, v0.16b, #1
    sri v2.16b, v0.16b, #7
    eor v0.16b, v0.16b, v2.16b
    ld1 {v1.16b}, [x2], #16
    eor v0.16b, v0.16b, v1.16b
    subs w3, w3, #1
    bne 1b
    ld1 {v16.16b-v19.16b}, [x1]
    tbl v2.16b, {v0.16b}, v16.16b
    tbl v3.16b, {v0.16b}, v17.16b
    tbl v4.16b, {v0.16b}, v18.16b
    tbl v5.16b, {v0.16b}, v19.16b
    eor v2.16b, v2.16b, v3.16b
    eor v4.16b, v4.16b, v5.16b
    eor v0.16b, v2.16b, v4.16b
    ld1 {v1.16b}, [x2]
    eor v0.16b, v0.16b, v1.16b
    st1 {v0.16b}, [x0]
    ret

aes128_decrypt_block_neon:
    ld1 {v0.16b}, [x0]
    add x2, x2, #160
    ld1 {v1.16b}, [x2], #-16
    eor v0.16b, v0.16b, v1.16b
    mov w3, #9
1:
    ld1 {v16.16b-v19.16b}, [x1], #64
    tbl v2.16b, {v0.16b}, v16.16b
    tbl v3.16b, {v0.16b}, v17.16b
    tbl v4.16b, {v0.16b}, v18.16b
    tbl v5.16b, {v0.16b}, v19.16b
    eor v2.16b, v2.16b, v3.16b
    eor v4.16b, v4.16b, v5.16b
    eor v0.16b, v2.16b, v4.16b
    shl v2.16b, v0.16b, #1
    sri v2.16b, v0.16b, #7
    eor v1.16b, v0.16b, v2.16b
    shl v2.16b, v1.16b, #1
    sri v2.16b, v1.16b, #7
    eor v0.16b, v0.16b, v2.16b
    ld1 {v1.16b}, [x2], #-16
    eor v0.16b, v0.16b, v1.16b
    subs w3, w3, #1
    bne 1b
    ld1 {v16.16b-v19.16b}, [x1]
    tbl v2.16b, {v0.16b}, v16.16b
    tbl v3.16b, {v0.16b}, v17.16b
    tbl v4.16b, {v0.16b}, v18.16b
    tbl v5.16b, {v0.16b}, v19.16b
    eor v2.16b, v2.16b, v3.16b
    eor v4.16b, v4.16b, v5.16b
    eor v0.16b, v2.16b, v4.16b
    ld1 {v1.16b}, [x2]
    eor v0.16b, v0.16b, v1.16b
    st1 {v0.16b}, [x0]
    ret

aes128_key_expand_neon:
    ld1 {v0.16b}, [x0]
    st1 {v0.16b}, [x1], #16
    adr x3, Rcon
    mov w4, #10
    mov w5, #0
1:
    dup v1.4s, w5
    ld1 {v2.s}[0], [x3], #4
    add w5, w5, #1
    ext v3.16b, v0.16b, v0.16b, #12
    aese v3.16b, v1.16b
    eor v0.16b, v0.16b, v3.16b
    eor v0.16b, v0.16b, v2.16b
    st1 {v0.16b}, [x1], #16
    ext v3.16b, v0.16b, v0.16b, #12
    aese v3.16b, v1.16b
    eor v0.16b, v0.16b, v3.16b
    st1 {v0.16b}, [x1], #16
    subs w4, w4, #1
    bne 1b
    ret

aes256_encrypt_block_neon:
    ld1 {v0.16b}, [x0]
    ld1 {v1.16b}, [x2], #16
    eor v0.16b, v0.16b, v1.16b
    mov w3, #13
1:
    ld1 {v16.16b-v19.16b}, [x1], #64
    tbl v2.16b, {v0.16b}, v16.16b
    tbl v3.16b, {v0.16b}, v17.16b
    tbl v4.16b, {v0.16b}, v18.16b
    tbl v5.16b, {v0.16b}, v19.16b
    eor v2.16b, v2.16b, v3.16b
    eor v4.16b, v4.16b, v5.16b
    eor v0.16b, v2.16b, v4.16b
    shl v2.16b, v0.16b, #1
    sri v2.16b, v0.16b, #7
    eor v0.16b, v0.16b, v2.16b
    ld1 {v1.16b}, [x2], #16
    eor v0.16b, v0.16b, v1.16b
    subs w3, w3, #1
    bne 1b
    ld1 {v16.16b-v19.16b}, [x1]
    tbl v2.16b, {v0.16b}, v16.16b
    tbl v3.16b, {v0.16b}, v17.16b
    tbl v4.16b, {v0.16b}, v18.16b
    tbl v5.16b, {v0.16b}, v19.16b
    eor v2.16b, v2.16b, v3.16b
    eor v4.16b, v4.16b, v5.16b
    eor v0.16b, v2.16b, v4.16b
    ld1 {v1.16b}, [x2]
    eor v0.16b, v0.16b, v1.16b
    st1 {v0.16b}, [x0]
    ret

aes256_decrypt_block_neon:
    ld1 {v0.16b}, [x0]
    add x2, x2, #224
    ld1 {v1.16b}, [x2], #-16
    eor v0.16b, v0.16b, v1.16b
    mov w3, #13
1:
    ld1 {v16.16b-v19.16b}, [x1], #64
    tbl v2.16b, {v0.16b}, v16.16b
    tbl v3.16b, {v0.16b}, v17.16b
    tbl v4.16b, {v0.16b}, v18.16b
    tbl v5.16b, {v0.16b}, v19.16b
    eor v2.16b, v2.16b, v3.16b
    eor v4.16b, v4.16b, v5.16b
    eor v0.16b, v2.16b, v4.16b
    shl v2.16b, v0.16b, #1
    sri v2.16b, v0.16b, #7
    eor v1.16b, v0.16b, v2.16b
    shl v2.16b, v1.16b, #1
    sri v2.16b, v1.16b, #7
    eor v0.16b, v0.16b, v2.16b
    ld1 {v1.16b}, [x2], #-16
    eor v0.16b, v0.16b, v1.16b
    subs w3, w3, #1
    bne 1b
    ld1 {v16.16b-v19.16b}, [x1]
    tbl v2.16b, {v0.16b}, v16.16b
    tbl v3.16b, {v0.16b}, v17.16b
    tbl v4.16b, {v0.16b}, v18.16b
    tbl v5.16b, {v0.16b}, v19.16b
    eor v2.16b, v2.16b, v3.16b
    eor v4.16b, v4.16b, v5.16b
    eor v0.16b, v2.16b, v4.16b
    ld1 {v1.16b}, [x2]
    eor v0.16b, v0.16b, v1.16b
    st1 {v0.16b}, [x0]
    ret

aes256_key_expand_neon:
    ld1 {v0.16b}, [x0]
    ld1 {v1.16b}, [x0, #16]
    st1 {v0.16b}, [x1], #16
    st1 {v1.16b}, [x1], #16
    adr x3, Rcon
    mov w4, #7
    mov w5, #0
1:
    dup v2.4s, w5
    ld1 {v3.s}[0], [x3], #4
    add w5, w5, #1
    ext v4.16b, v1.16b, v1.16b, #12
    aese v4.16b, v2.16b
    eor v0.16b, v0.16b, v4.16b
    eor v0.16b, v0.16b, v3.16b
    st1 {v0.16b}, [x1], #16
    ext v4.16b, v0.16b, v0.16b, #12
    aese v4.16b, v2.16b
    eor v1.16b, v1.16b, v4.16b
    st1 {v1.16b}, [x1], #16
    subs w4, w4, #1
    bne 1b
    ret

.global aes_ctr_encrypt_neon
aes_ctr_encrypt_neon:
    ld1 {v0.16b}, [x1]
    mov w4, #0
1:
    cmp w4, w2
    bge 3f
    mov v1.16b, v0.16b
    mov x5, x0
    add x5, x5, w4, uxtw
    ld1 {v2.16b}, [x5]
    ld1 {v3.16b}, [x3], #16
    eor v1.16b, v1.16b, v3.16b
    mov w6, #9
2:
    ld1 {v16.16b-v19.16b}, [x6], #64
    tbl v4.16b, {v1.16b}, v16.16b
    tbl v5.16b, {v1.16b}, v17.16b
    tbl v6.16b, {v1.16b}, v18.16b
    tbl v7.16b, {v1.16b}, v19.16b
    eor v4.16b, v4.16b, v5.16b
    eor v6.16b, v6.16b, v7.16b
    eor v1.16b, v4.16b, v6.16b
    shl v4.16b, v1.16b, #1
    sri v4.16b, v1.16b, #7
    eor v1.16b, v1.16b, v4.16b
    ld1 {v3.16b}, [x3], #16
    eor v1.16b, v1.16b, v3.16b
    subs w6, w6, #1
    bne 2b
    ld1 {v16.16b-v19.16b}, [x6]
    tbl v4.16b, {v1.16b}, v16.16b
    tbl v5.16b, {v1.16b}, v17.16b
    tbl v6.16b, {v1.16b}, v18.16b
    tbl v7.16b, {v1.16b}, v19.16b
    eor v4.16b, v4.16b, v5.16b
    eor v6.16b, v6.16b, v7.16b
    eor v1.16b, v4.16b, v6.16b
    ld1 {v3.16b}, [x3]
    eor v1.16b, v1.16b, v3.16b
    eor v2.16b, v2.16b, v1.16b
    st1 {v2.16b}, [x5]
    add v0.16b, v0.16b, v31.16b
    add w4, w4, #16
    b 1b
3:
    ret

.global aes_ecb_encrypt_neon
aes_ecb_encrypt_neon:
    lsr w2, w2, #4
1:
    cbz w2, 3f
    ld1 {v0.16b}, [x0]
    mov x3, x1
    mov w4, #9
    ld1 {v1.16b}, [x3], #16
    eor v0.16b, v0.16b, v1.16b
2:
    ld1 {v16.16b-v19.16b}, [x3], #64
    tbl v2.16b, {v0.16b}, v16.16b
    tbl v3.16b, {v0.16b}, v17.16b
    tbl v4.16b, {v0.16b}, v18.16b
    tbl v5.16b, {v0.16b}, v19.16b
    eor v2.16b, v2.16b, v3.16b
    eor v4.16b, v4.16b, v5.16b
    eor v0.16b, v2.16b, v4.16b
    shl v2.16b, v0.16b, #1
    sri v2.16b, v0.16b, #7
    eor v0.16b, v0.16b, v2.16b
    ld1 {v1.16b}, [x3], #16
    eor v0.16b, v0.16b, v1.16b
    subs w4, w4, #1
    bne 2b
    ld1 {v16.16b-v19.16b}, [x3]
    tbl v2.16b, {v0.16b}, v16.16b
    tbl v3.16b, {v0.16b}, v17.16b
    tbl v4.16b, {v0.16b}, v18.16b
    tbl v5.16b, {v0.16b}, v19.16b
    eor v2.16b, v2.16b, v3.16b
    eor v4.16b, v4.16b, v5.16b
    eor v0.16b, v2.16b, v4.16b
    ld1 {v1.16b}, [x3, #64]
    eor v0.16b, v0.16b, v1.16b
    st1 {v0.16b}, [x0], #16
    sub w2, w2, #1
    b 1b
3:
    ret

/*
		test.c

void aes128_encrypt_block_neon(uint8_t* block, const uint8_t* te_tables, const uint8_t* key_schedule);
void aes128_decrypt_block_neon(uint8_t* block, const uint8_t* td_tables, const uint8_t* key_schedule);
void aes128_key_expand_neon(const uint8_t* key, uint8_t* key_schedule);

void aes256_encrypt_block_neon(uint8_t* block, const uint8_t* te_tables, const uint8_t* key_schedule);
void aes256_decrypt_block_neon(uint8_t* block, const uint8_t* td_tables, const uint8_t* key_schedule);
void aes256_key_expand_neon(const uint8_t* key, uint8_t* key_schedule);

void aes_ctr_encrypt_neon(uint8_t* data, const uint8_t* counter, uint32_t length, const uint8_t* key_schedule);
void aes_ecb_encrypt_neon(uint8_t* data, const uint8_t* key_schedule, uint32_t length);

uint8_t te_tables[4*256*4];
uint8_t td_tables[4*256*4];

void init_tables() {
    for(int i=0;i<256;i++){
        uint8_t s = sbox[i];
        ((uint32_t*)te_tables)[i] = (gf_mul[s][0]<<24)|(s<<16)|(s<<8)|gf_mul[s][3];
        ((uint32_t*)te_tables)[256+i] = (gf_mul[s][1]<<24)|(gf_mul[s][0]<<16)|(s<<8)|s;
        ((uint32_t*)te_tables)[512+i] = (gf_mul[s][2]<<24)|(gf_mul[s][1]<<16)|(gf_mul[s][0]<<8)|s;
        ((uint32_t*)te_tables)[768+i] = (gf_mul[s][3]<<24)|(gf_mul[s][2]<<16)|(gf_mul[s][1]<<8)|gf_mul[s][0];
        
        uint8_t inv_s = inv_sbox[i];
        ((uint32_t*)td_tables)[i] = (gf_mul[inv_s][5]<<24)|(gf_mul[inv_s][3]<<16)|(gf_mul[inv_s][4]<<8)|gf_mul[inv_s][2];
        ((uint32_t*)td_tables)[256+i] = (gf_mul[inv_s][2]<<24)|(gf_mul[inv_s][5]<<16)|(gf_mul[inv_s][3]<<8)|gf_mul[inv_s][4];
        ((uint32_t*)td_tables)[512+i] = (gf_mul[inv_s][4]<<24)|(gf_mul[inv_s][2]<<16)|(gf_mul[inv_s][5]<<8)|gf_mul[inv_s][3];
        ((uint32_t*)td_tables)[768+i] = (gf_mul[inv_s][3]<<24)|(gf_mul[inv_s][4]<<16)|(gf_mul[inv_s][2]<<8)|gf_mul[inv_s][5];
    }
}

*/
