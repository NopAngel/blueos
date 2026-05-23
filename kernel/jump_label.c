#include <kernel/jump_label.h>
#include <stdint.h>

void patch_jump_label(struct jump_entry *entry, int enable) {
    uint32_t *insn_addr = (uint32_t *)entry->code;
    uint32_t new_insn;

    if (enable) {
        int32_t offset = (int32_t)entry->target - (int32_t)entry->code;

        uint32_t imm20  = (offset >> 20) & 0x1;
        uint32_t imm10_1 = (offset >> 1)  & 0x3FF;
        uint32_t imm11   = (offset >> 11) & 0x1;
        uint32_t imm19_12 = (offset >> 12) & 0xFF;

        new_insn = 0x6F | (imm20 << 31) | (imm10_1 << 21) | (imm11 << 20) | (imm19_12 << 12);
    } else {
        new_insn = 0x00000013; 
    }

    *insn_addr = new_insn;

    asm volatile ("fence.i" : : : "memory");
}