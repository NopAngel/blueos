#ifndef HPET_H
#define HPET_H

#include <stdint.h>

struct acpi_sdt_header {
  char signature[4];
  uint32_t length;
  uint8_t revision;
  uint8_t checksum;
  char oem_id[6];
  char oem_table_id[8];
  uint32_t oem_revision;
  uint32_t creator_id;
  uint32_t creator_revision;
} __attribute__((packed));

typedef struct {
  struct acpi_sdt_header header;
  uint32_t event_timer_block_id;
  struct {
    uint8_t address_space_id;
    uint8_t register_bit_width;
    uint8_t register_bit_offset;
    uint8_t reserved;
    uint64_t address;
  } base_address;
  uint8_t hpet_number;
  uint16_t main_counter_minimum_tick;
  uint8_t page_protection;
} __attribute__((packed)) hpet_table_t;

void hpet_init(hpet_table_t *table);
uint64_t hpet_get_nanos();

#endif