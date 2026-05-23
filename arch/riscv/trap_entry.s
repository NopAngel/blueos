.align 4
.global trap_entry
trap_entry:
    addi sp, sp, -144

    sw ra, 4(sp)
    sw gp, 12(sp)
    sw tp, 16(sp)
    sw t0, 20(sp)
    sw t6, 124(sp)

    csrr t0, scause
    sw t0, 128(sp)
    csrr t0, stval
    sw t0, 132(sp)
    csrr t0, sepc
    sw t0, 136(sp)
    csrr t0, sstatus
    sw t0, 140(sp)
    mv a0, sp
    call main_trap_handler

    addi sp, sp, 144
    sret

.align 4
trap_vector:
    addi sp, sp, -256
    sw ra, 0(sp)
    sw t0, 4(sp)

    mv a0, sp        
    call handle_trap

    lw ra, 0(sp)
    addi sp, sp, 256
    mret