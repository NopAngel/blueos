.section .text._start
.global _start
_start:
    la sp, __stack_top    
    add s0, sp, zero      
    jal zero, k_main        
loop:	j loop              
													
.section .data
.space 1024*8             
.align 16                
__stack_top:             

.global enable_paging
enable_paging:
    srli a0, a0, 12           
    li t0, 0x80000000         
    or a0, a0, t0
    csrw satp, a0             
    sfence.vma                
    ret


.global restore_system_context
restore_system_context:
    lw   t0, 60(a0)
    csrw satp, t0
    sfence.vma        

    lw ra,  0(a0)
    lw sp,  4(a0)
    # ...
    
    li a0, 1
    ret