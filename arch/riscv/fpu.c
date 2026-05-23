#include <stdint.h>

#define MSTATUS_FS_MASK  (3 << 13)
#define MSTATUS_FS_DIRTY (3 << 13)
#define MSTATUS_FS_CLEAN (2 << 13)

void fpu_init() {
    uintptr_t mstatus;
    
    asm volatile ("csrr %0, mstatus" : "=r"(mstatus));

    mstatus = (mstatus & ~MSTATUS_FS_MASK) | MSTATUS_FS_CLEAN;
    asm volatile ("csrw mstatus, %0" : : "r"(mstatus));
    
    asm volatile ("csrw fcsr, x0");
}