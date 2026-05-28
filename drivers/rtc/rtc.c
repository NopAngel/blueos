#include <kernel/rtc.h>
#include <kernel/printk.h>

#if defined(x86)
#include <kernel/io.h>    /* Required for inb/outb on x86 */
#define RTC_ADDR 0x70
#define RTC_DATA 0x71
#elif defined(RISCV)
/* RISC-V uses Memory Mapped I/O (MMIO) instead of Port I/O */
#define RTC_MMIO_BASE 0x101000  /* Standard QEMU Virt Goldfish RTC address */
#endif

/**
 * get_rtc_register: Reads a specific CMOS/RTC register.
 * @reg: The register number to read.
 */
uint8_t get_rtc_register(int reg) {
#if defined(x86)
    /* Select the register via port 0x70 and read from 0x71 */
    outb(RTC_ADDR, (uint8_t)reg);
    return inb(RTC_DATA);

#elif defined(RISCV)
    /* On RISC-V, we usually read the offset from the MMIO base */
    volatile uint32_t *rtc_ptr = (volatile uint32_t *)(RTC_MMIO_BASE + (reg * 4));
    return (uint8_t)(*rtc_ptr);

#else
    #error "Architecture not supported in rtc.c"
#endif
}

/**
 * get_rtc_second: Returns the current system second.
 */
uint8_t get_rtc_second(void) {
    /* Register 0x00 is typically the seconds register in most RTCs */
    return get_rtc_register(0x00);
}

/**
 * rtc_init: Hardware-specific initialization for the clock.
 */
void rtc_init(void) {
#if defined(x86)
    pr_info("RTC: Hardware initialized via Port I/O (x86)\n");
#elif defined(RISCV)
    pr_info("RTC: Hardware initialized via MMIO (RISC-V)\n");
#endif
}

// device_initcall(rtc_init);
