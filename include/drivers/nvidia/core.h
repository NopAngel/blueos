#ifndef NVIDIA_MGMT_H
#define NVIDIA_MGMT_H

typedef struct {
    uint32_t vendor_id;
    uint32_t device_id;
    uint32_t vram_size;
    uint32_t core_clock;
    uint32_t memory_clock;
    uint32_t temperature;
    uint32_t fan_speed;
    uint32_t load_percentage;
    char device_name[64];
    char family_name[32];
} nvidia_gpu_info;

typedef struct {
    uint32_t current_core_clock;
    uint32_t current_memory_clock;
    uint32_t target_fan_speed;
    uint32_t fan_control_mode;
} nvidia_config;

int nvidia_mgmt_detect(void);
void nvidia_mgmt_init(void);
void nvidia_print_info(void);
void nvidia_set_fan_speed(uint8_t percentage);
void nvidia_set_clocks(uint32_t core_mhz, uint32_t mem_mhz);
uint32_t nvidia_get_temperature(void);
uint32_t nvidia_get_fan_speed(void);
uint32_t nvidia_get_clock_core(void);
uint32_t nvidia_get_clock_memory(void);

#endif