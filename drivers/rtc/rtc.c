
#include <drivers/rtc.h>
#include <kernel/printk.h>

#if defined(x86)
#include <kernel/io.h>
#define RTC_ADDR 0x70
#define RTC_DATA 0x71
#elif defined(RISCV)
// MMIO para Goldfish RTC en QEMU Virt
#define RTC_MMIO_LOW 0x101000
#define RTC_MMIO_HIGH 0x101004
#endif

// Helper para convertir BCD a Binario (Fundamental en x86)
static uint8_t bcd_to_bin(uint8_t val) {
  return (val & 0x0F) + ((val / 16) * 10);
}

uint8_t get_rtc_register(int reg) {
#if defined(x86)
  outb(RTC_ADDR, (uint8_t)reg);
  return inb(RTC_DATA);
#elif defined(RISCV)
  return 0;
#endif
}

static void unix_to_date(uint64_t seconds, rtc_time_t *time) {
  uint64_t days = seconds / 86400;
  uint32_t rem_seconds = seconds % 86400;

  time->hour = rem_seconds / 3600;
  time->minute = (rem_seconds % 3600) / 60;
  time->second = rem_seconds % 60;

  uint32_t y = 1970;
  while (days >=
         (uint64_t)(365 + (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0)))) {
    days -= (365 + (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0)));
    y++;
  }
  time->year = y;
  time->day = (days % 31) + 1;
  time->month = 1;
}
/**
 * get_rtc_time: Llena la estructura con la fecha completa
 */
void get_rtc_time(rtc_time_t *time) {
#if defined(x86)
  outb(RTC_ADDR, 0x0A);
  while (inb(RTC_DATA) & 0x80)
    ;

  time->second = bcd_to_bin(get_rtc_register(0x00));
  time->minute = bcd_to_bin(get_rtc_register(0x02));
  time->hour = bcd_to_bin(get_rtc_register(0x04));
  time->day = bcd_to_bin(get_rtc_register(0x07));
  time->month = bcd_to_bin(get_rtc_register(0x08));
  time->year = bcd_to_bin(get_rtc_register(0x09)) + 2000;
  time->msec = 0;

#elif defined(RISCV)

  volatile uint32_t *low = (volatile uint32_t *)RTC_MMIO_LOW;
  volatile uint32_t *high = (volatile uint32_t *)RTC_MMIO_HIGH;

  uint64_t nsec = ((uint64_t)*high << 32) | *low;
  uint64_t sec = nsec / 1000000000;

  time->msec = (nsec / 1000000) % 1000;

  time->second = sec % 60;
  time->minute = (sec / 60) % 60;
  time->hour = (sec / 3600) % 24;
#endif
}

void get_local_time(rtc_time_t *time) {
  get_rtc_time(time);

  int offset = -4;

  int local_hour = (int)time->hour + offset;

  if (local_hour < 0) {
    local_hour += 24;
    time->day--;
  } else if (local_hour >= 24) {
    local_hour -= 24;
    time->day++;
  }

  time->hour = (uint8_t)local_hour;
}
