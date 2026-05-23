#include <stdint.h>
#include <lib/string.h>

#define RV_REG_A0 10
#define RV_REG_A1 11


void emit_insn(uint32_t *target, uint32_t insn) {
    *target = insn;
}


void bpf_jit_compile(uint64_t *bpf_prog, int len, uint32_t *out_bin) {
    uint32_t *ptr = out_bin;

    for (int i = 0; i < len; i++) {
        uint64_t insn = bpf_prog[i];
        uint8_t code = insn & 0xFF;

        switch (code) {
            case 0xb7: // BPF_ALU64 | BPF_MOV | BPF_K
                {
                    uint8_t dst = (insn >> 8) & 0xF;
                    int32_t imm = (int32_t)(insn >> 32);
                    
                    uint32_t rv_insn = 0x00000013 | ((10 + dst) << 7) | (imm << 20);
                    emit_insn(ptr++, rv_insn);
                }
                break;

            case 0x95: // BPF_JMP | BPF_EXIT
                emit_insn(ptr++, 0x00008067);
                break;
                
            default:
                emit_insn(ptr++, 0x00000013);
                break;
        }
    }
}