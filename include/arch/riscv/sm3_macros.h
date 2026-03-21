#ifndef SM3_MACROS_H
#define SM3_MACROS_H

#define ENABLE_SM3_EXT  asm volatile (".option push\n\t.option arch, +zkh")
#define DISABLE_SM3_EXT asm volatile (".option pop")

static inline uint32_t riscv_sm3p0(uint32_t x) {
    uint32_t rd;
    ENABLE_SM3_EXT;
    asm volatile ("sm3p0 %0, %1" : "=r"(rd) : "r"(x));
    DISABLE_SM3_EXT;
    return rd;
}

static inline uint32_t riscv_sm3p1(uint32_t x) {
    uint32_t rd;
    ENABLE_SM3_EXT;
    asm volatile ("sm3p1 %0, %1" : "=r"(rd) : "r"(x));
    DISABLE_SM3_EXT;
    return rd;
}

#endif