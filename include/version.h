/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _BLUEOS_VERSION_H
#define _BLUEOS_VERSION_H


#define BLUEOS_MAJOR     2
#define BLUEOS_MINOR     1
#define BLUEOS_PATCH     0

#define KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
#define BLUEOS_VERSION_CODE KERNEL_VERSION(BLUEOS_MAJOR, BLUEOS_MINOR, BLUEOS_PATCH)

#if defined(__x86_64__)
    #define BLUEOS_ARCH "x86_64"
#elif defined(__i386__) || defined(__i686__)
    #define BLUEOS_ARCH "i386"
#else
    #define BLUEOS_ARCH "unknown"
#endif

#define COMPILER_INFO "gcc version " __VERSION__


#define BLUEOS_NAME "BlueOS"


#define UTS_RELEASE    "2.1.0-blueos"
#define UTS_VERSION    "#1 SMP PREEMPT " __DATE__ " " __TIME__
#define UTS_MACHINE    BLUEOS_ARCH

static inline const char* get_kernel_banner(void)
{
    return "BlueOS version " UTS_RELEASE " (" COMPILER_INFO ") " UTS_VERSION;
}


#define I_VERSION_QUERIED_BIT (1ULL << 0)
#define I_VERSION_INC_STEP    (1ULL << 1)

#endif /* _BLUEOS_VERSION_H */