.section .text
.global save_system_context

save_system_context:
    sw ra,  0(a0)
    sw sp,  4(a0)
    sw s0,  8(a0)
    sw s1,  12(a0)
    sw s2,  16(a0)
    sw s3,  20(a0)
    sw s4,  24(a0)
    sw s5,  28(a0)
    sw s6,  32(a0)
    sw s7,  36(a0)
    sw s8,  40(a0)
    sw s9,  44(a0)
    sw s10, 48(a0)
    sw s11, 52(a0)
    
    csrr t0, satp
    sw   t0, 56(a0)
    csrr t1, mstatus
    sw   t1, 60(a0)

    li a0, 0    
    ret