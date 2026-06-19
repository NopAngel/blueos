#ifndef FAT16_H
#define FAT16_H

#include <stdint.h>

typedef struct {
  uint8_t name[8];
  uint8_t ext[3];
  uint8_t attributes;
  uint8_t reserved;
  uint8_t creation_time_ms;
  uint16_t creation_time;
  uint16_t creation_date;
  uint16_t last_access_date;
  uint16_t cluster_high;
  uint16_t m_time;
  uint16_t m_date;
  uint16_t cluster_low;
  uint32_t file_size;
} __attribute__((packed)) fat_entry_t;

typedef struct {
  uint8_t jmp[3];
  char oem[8];
  uint16_t bytes_per_sector;
  uint8_t sectors_per_cluster;
  uint16_t reserved_sectors;
  uint8_t fat_count;
  uint16_t root_entries;
  uint16_t total_sectors_16;
  uint8_t media_type;
  uint16_t fat_size_16;
  uint16_t sectors_per_track;
  uint16_t num_heads;
  uint32_t hidden_sectors;
  uint32_t total_sectors_32;
  uint8_t drive_number;
  uint8_t reserved1;
  uint8_t boot_signature;
  uint32_t volume_id;
  char volume_label[11];
  char fs_type[8];
} __attribute__((packed)) fat16_bpb_t;

#endif
