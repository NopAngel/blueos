/*
 * include/kernel/ebpf.h
 * Native eBPF Virtual Machine Definition for BlueOS
 * Architecture agnostic, strictly in English.
 */

#ifndef _BLUEOS_EBPF_H
#define _BLUEOS_EBPF_H

#include <stdint.h>
#include <stddef.h>

/* eBPF Instruction Structure (8 bytes aligned) */
struct ebpf_insn {
    uint8_t  code;    /* Opcode */
    uint8_t  dst_reg:4; /* Destination register (4 bits) */
    uint8_t  src_reg:4; /* Source register (4 bits) */
    int16_t  off;     /* Signed offset */
    int32_t  imm;     /* Signed immediate value */
};

/* eBPF Register Layout (64-bit architecture inside the VM) */
#define EBPF_REG_0  0  /* Return value from helper function / program exit */
#define EBPF_REG_1  1  /* Argument 1 for helper functions / Context pointer */
#define EBPF_REG_2  2  /* Argument 2 for helper functions */
#define EBPF_REG_3  3  /* Argument 3 for helper functions */
#define EBPF_REG_4  4  /* Argument 4 for helper functions */
#define EBPF_REG_5  5  /* Argument 5 for helper functions */
#define EBPF_REG_6  6  /* Callee saved register 1 */
#define EBPF_REG_7  7  /* Callee saved register 2 */
#define EBPF_REG_8  8  /* Callee saved register 3 */
#define EBPF_REG_9  9  /* Callee saved register 4 */
#define EBPF_REG_10 10 /* Frame pointer (Read-only to access stack) */
#define EBPF_MAX_REGS 11

/* eBPF Core Opcodes Classes */
#define EBPF_CLS_LD    0x00
#define EBPF_CLS_LDX   0x01
#define EBPF_CLS_ST    0x02
#define EBPF_CLS_STX   0x03
#define EBPF_CLS_ALU   0x04
#define EBPF_CLS_JMP   0x05
#define EBPF_CLS_ALU64 0x07

/* eBPF ALU Operations */
#define EBPF_ALU_ADD   0x00
#define EBPF_ALU_SUB   0x10
#define EBPF_ALU_MUL   0x20
#define EBPF_ALU_DIV   0x30
#define EBPF_ALU_OR    0x40
#define EBPF_ALU_AND   0x50
#define EBPF_ALU_LSH   0x60
#define EBPF_ALU_RSH   0x70
#define EBPF_ALU_NEG   0x80
#define EBPF_ALU_MOD   0x90
#define EBPF_ALU_XOR   0xa0
#define EBPF_ALU_MOV   0xb0

/* Source Modifier Flags */
#define EBPF_K         0x00 /* Immediate value */
#define EBPF_X         0x08 /* Source Register */

/* Execution Context for the Interpreter */
struct ebpf_vm {
    const struct ebpf_insn *insns;
    uint32_t num_insns;
    /* Future expansion: array of helper functions hooks */
    uint64_t (*helpers[16])(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
};

/* Function declarations */
int ebpf_register_helper(struct ebpf_vm *vm, uint32_t id, void *fn);
uint64_t ebpf_run(const struct ebpf_vm *vm, void *ctx);
void ebpf_init(void);

#endif /* _BLUEOS_EBPF_H */