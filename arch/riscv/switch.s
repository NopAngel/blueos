.section .text
.option push
.option arch, +d   
.global switch_to_task

switch_to_task:

    addi sp, sp, -320

   
    sw ra,  0(sp)
    sw s0,  4(sp)
    sw s1,  8(sp)
    # ... (s2 a s11) ...
    sw s11, 48(sp)

    fsd f0,  64(sp)
    fsd f1,  72(sp)
    fsd f2,  80(sp)

    fsd f31, 312(sp)

    frcsr t0
    sw    t0, 316(sp)

    sw sp, 0(a0)

    mv sp, a1

    lw ra,  0(sp)
    lw s0,  4(sp)
    # ... (s1 a s11) ...
    
    fld f0,  64(sp)
    fld f1,  72(sp)
    fld f31, 312(sp)

    lw    t0, 316(sp)
    fscsr t0

    addi sp, sp, 320
    ret


.global switch_to
switch_to:

    addi sp, sp, -320

    sw ra,  0(sp)
    sw s0,  4(sp)
    sw s1,  8(sp)
    sw s2,  12(sp)
    sw s3,  16(sp)
    sw s4,  20(sp)
    sw s5,  24(sp)
    sw s6,  28(sp)
    sw s7,  32(sp)
    sw s8,  36(sp)
    sw s9,  40(sp)
    sw s10, 44(sp)
    sw s11, 48(sp)

    fsd f0,  64(sp)
    fsd f1,  72(sp)
    fsd f2,  80(sp)
    fsd f3,  88(sp)
    fsd f4,  96(sp)
    fsd f5,  104(sp)
    fsd f6,  112(sp)
    fsd f7,  120(sp)
    fsd f8,  128(sp)
    fsd f9,  136(sp)
    fsd f10, 144(sp)
    fsd f11, 152(sp)
    fsd f12, 160(sp)
    fsd f13, 168(sp)
    fsd f14, 176(sp)
    fsd f15, 184(sp)
    fsd f16, 192(sp)
    fsd f17, 200(sp)
    fsd f18, 208(sp)
    fsd f19, 216(sp)
    fsd f20, 224(sp)
    fsd f21, 232(sp)
    fsd f22, 240(sp)
    fsd f23, 248(sp)
    fsd f24, 256(sp)
    fsd f25, 264(sp)
    fsd f26, 272(sp)
    fsd f27, 280(sp)
    fsd f28, 288(sp)
    fsd f29, 296(sp)
    fsd f30, 304(sp)
    fsd f31, 312(sp)

    frcsr t0
    sw    t0, 316(sp)

    sw sp, 0(a0)

    lw sp, 0(a1)

    lw ra,  0(sp)
    lw s0,  4(sp)
    lw s1,  8(sp)
    lw s2,  12(sp)
    lw s3,  16(sp)
    lw s4,  20(sp)
    lw s5,  24(sp)
    lw s6,  28(sp)
    lw s7,  32(sp)
    lw s8,  36(sp)
    lw s9,  40(sp)
    lw s10, 44(sp)
    lw s11, 48(sp)

    fld f0,  64(sp)
    fld f1,  72(sp)
    fld f2,  80(sp)
    fld f3,  88(sp)
    fld f4,  96(sp)
    fld f5,  104(sp)
    fld f6,  112(sp)
    fld f7,  120(sp)
    fld f8,  128(sp)
    fld f9,  136(sp)
    fld f10, 144(sp)
    fld f11, 152(sp)
    fld f12, 160(sp)
    fld f13, 168(sp)
    fld f14, 176(sp)
    fld f15, 184(sp)
    fld f16, 192(sp)
    fld f17, 200(sp)
    fld f18, 208(sp)
    fld f19, 216(sp)
    fld f20, 224(sp)
    fld f21, 232(sp)
    fld f22, 240(sp)
    fld f23, 248(sp)
    fld f24, 256(sp)
    fld f25, 264(sp)
    fld f26, 272(sp)
    fld f27, 280(sp)
    fld f28, 288(sp)
    fld f29, 296(sp)
    fld f30, 304(sp)
    fld f31, 312(sp)

    lw    t0, 316(sp)
    fscsr t0

    addi sp, sp, 320
    ret

.option pop