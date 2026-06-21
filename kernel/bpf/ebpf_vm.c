/*
 * kernel/bpf/ebpf_vm.c
 * Core eBPF Interpreter Execution Engine
 * Pure C, strictly in English.
 */

#include <kernel/ebpf.h>
#include <kernel/timer.h>
#include <kernel/printk.h>

struct ebpf_vm g_kernel_vm;

int ebpf_register_helper(struct ebpf_vm *vm, uint32_t id, void *fn) {
    if (!vm || id >= 16) {
        printk("<3> EBPF: Failed to register helper ID %d\n", id);
        return -1;
    }
    vm->helpers[id] = fn;
    return 0;
}

uint64_t ebpf_run(const struct ebpf_vm *vm, void *ctx) {
    if (!vm || !vm->insns) {
        return 0;
    }

    /* Virtual Machine Registers initialized to zero */
    uint64_t regs[EBPF_MAX_REGS] = {0};
    
    /* Explicitly simulate stack memory for the program context */
    uint8_t stack[512] = {0};

    /* R1 receives the initial pointer context */
    regs[EBPF_REG_1] = (uintptr_t)ctx;
    
    /* R10 points to the top of the stack space */
    regs[EBPF_REG_10] = (uintptr_t)(stack + sizeof(stack));

    uint32_t pc = 0;

    while (pc < vm->num_insns) {
        struct ebpf_insn insn = vm->insns[pc];
        uint8_t op = insn.code;
        pc++; /* Advance program counter */

        switch (op) {
            /* === ALU64 Immediate Class === */
            case (EBPF_CLS_ALU64 | EBPF_ALU_MOV | EBPF_K):
                regs[insn.dst_reg] = (uint64_t)insn.imm;
                break;

            case (EBPF_CLS_ALU64 | EBPF_ALU_ADD | EBPF_K):
                regs[insn.dst_reg] += (uint64_t)insn.imm;
                break;

            case (EBPF_CLS_ALU64 | EBPF_ALU_SUB | EBPF_K):
                regs[insn.dst_reg] -= (uint64_t)insn.imm;
                break;

            /* === ALU64 Register Class === */
            case (EBPF_CLS_ALU64 | EBPF_ALU_MOV | EBPF_X):
                regs[insn.dst_reg] = regs[insn.src_reg];
                break;

            case (EBPF_CLS_ALU64 | EBPF_ALU_ADD | EBPF_X):
                regs[insn.dst_reg] += regs[insn.src_reg];
                break;

            /* === Memory Access (STX: Store Register into Memory) === */
            case (EBPF_CLS_STX | 0x10): /* Store double word (64-bit) */
                *(uint64_t *)(uintptr_t)(regs[insn.dst_reg] + insn.off) = regs[insn.src_reg];
                break;

            /* === Jump Operations === */
            case (EBPF_CLS_JMP | 0x90): /* CALL instruction to helper routines */
                if (insn.imm >= 0 && insn.imm < 16 && vm->helpers[insn.imm]) {
                    /* Call registered function pointer passing VM registers as arguments */
                    regs[EBPF_REG_0] = vm->helpers[insn.imm](
                        regs[EBPF_REG_1], regs[EBPF_REG_2], 
                        regs[EBPF_REG_3], regs[EBPF_REG_4], regs[EBPF_REG_5]
                    );
                } else {
                    printk("<3> EBPF: Invalid kernel helper call ID %d at PC %d\n", insn.imm, pc - 1);
                    return 0;
                }
                break;

            case (EBPF_CLS_JMP | 0x10): /* EXIT program execution */
                return regs[EBPF_REG_0];

            default:
                printk("<4> EBPF: Unknown instruction opcode 0x%x at PC %d\n", op, pc - 1);
                return 0;
        }
    }

    return regs[EBPF_REG_0];
}

static uint64_t bpf_helper_printk(uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5) {
    const char *fmt = (const char *)(uintptr_t)arg1;
    return (uint64_t)printk(fmt, arg2, arg3, arg4, arg5);
}

static uint64_t bpf_helper_get_uptime(uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5) {
    (void)arg1; (void)arg2; (void)arg3; (void)arg4; (void)arg5;
    return timer_get_ms();
}

/**
 * ebpf_init - Subsystem initialization entrypoint called by init_all()
 */
void ebpf_init(void) {
    /* Clear the global virtual machine state */
    for (int i = 0; i < 16; i++) {
        g_kernel_vm.helpers[i] = NULL;
    }
    g_kernel_vm.insns = NULL;
    g_kernel_vm.num_insns = 0;

    /* Register core built-in kernel helper routines */
    ebpf_register_helper(&g_kernel_vm, 0, bpf_helper_printk);
    ebpf_register_helper(&g_kernel_vm, 1, bpf_helper_get_uptime);

    boot_msg("EBPF","Interpreter subsystem core variables loaded.", 0);
    boot_msg("EBPF", "Core helper ID 0 (printk) and ID 1 (get_uptime) linked.", 0);
}