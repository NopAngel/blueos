#ifndef _BLUEOS_VERSION_H
#define _BLUEOS_VERSION_H

/* --- Version Core --- */
#define BLUEOS_MAJOR    2
#define BLUEOS_MINOR    2
#define BLUEOS_PATCH    2

/* Helper to compare versions in code: #if BLUEOS_VERSION_CODE >= KERNEL_VERSION(2,3,0) */
#define KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
#define BLUEOS_VERSION_CODE KERNEL_VERSION(BLUEOS_MAJOR, BLUEOS_MINOR, BLUEOS_PATCH)

/* --- Architecture Detection --- */
#if defined(__riscv)
    #if __riscv_xlen == 64
        #define BLUEOS_ARCH "riscv64"
    #else
        #define BLUEOS_ARCH "riscv32"
    #endif
#elif defined(__x86__)
    #define BLUEOS_ARCH "x86"
#elif defined(__x86_64__)
    #define BLUEOS_ARCH "x86_64"
#elif defined(__mips__)
    #define BLUEOS_ARCH "mips"
#elif defined(__or1k__)
    #define BLUEOS_ARCH "openrisc"
#else
    #define BLUEOS_ARCH "unknown"
#endif

/* --- Metadata --- */
#define BLUEOS_NAME     "BlueOS"
#define COMPILER_INFO   "gcc " __VERSION__
#define UTS_RELEASE     "2.2.1-blueos"

#define UTS_VERSION    "#1 SMP PREEMPT 2026-06-01 19:48:36"
#define UTS_MACHINE     BLUEOS_ARCH

/* --- Helpful Macros --- */
/* Check if we are on a 64-bit system */
#if defined(__LP64__) || defined(_LP64) || defined(__riscv_xlen) && __riscv_xlen == 64
    #define BLUEOS_64BIT 1
#else
    #define BLUEOS_64BIT 0
#endif

/* --- Kernel Banner Function --- */
/* Marked as 'static inline' to avoid multiple definition errors when included */
static inline const char* get_kernel_banner(void) {
    return BLUEOS_NAME " version " UTS_RELEASE " (" COMPILER_INFO ") " \
           "Arch: " BLUEOS_ARCH " " UTS_VERSION;
}

#endif /* _BLUEOS_VERSION_H */
