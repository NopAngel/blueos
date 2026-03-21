#ifndef AES_MACROS_H
#define AES_MACROS_H

// rd, rs1, rs2, bs(?)
#define AES32ESI(rd, rs1, rs2, bs) \
    asm volatile ("aes32esi %0, %1, %2, %3" : "=r"(rd) : "r"(rs1), "r"(rs2), "i"(bs))

#define AES32ESMI(rd, rs1, rs2, bs) \
    asm volatile ("aes32esmi %0, %1, %2, %3" : "=r"(rd) : "r"(rs1), "r"(rs2), "i"(bs))

#endif