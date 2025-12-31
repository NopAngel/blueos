#include <include/drivers/nvidia/core.h">
#include <include/types.h">

static nvidia_gpu_info gpu_info;
static nvidia_config current_config;
static uint8_t nvidia_detected = 0;

static const char* device_names[] = {
    "GeForce GTX 480",
    "GeForce GTX 280",
    "GeForce 9800 GTX",
    "GeForce 8800 GTX",
    "GeForce 7800 GTX",
    "GeForce 6800",
    "GeForce FX 5200",
    "GeForce4 Ti 4600",
    "GeForce3",
    "GeForce2 GTS",
    "GeForce 256",
    "RIVA TNT2",
    "RIVA TNT",
    "RIVA 128",
    NULL
};

static void simple_strcpy(char* dest, const char* src) {
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
}

static int simple_strlen(const char* str) {
    int len = 0;
    while (*str++) len++;
    return len;
}

static void simple_strcat(char* dest, const char* src) {
    while (*dest) dest++;
    while (*src) *dest++ = *src++;
    *dest = '\0';
}

void nvidia_mgmt_init(void) {
    nvidia_mgmt_detect();
}


void nvidia_set_fan_speed(uint8_t percentage) {
    if (!nvidia_detected) return;

    if (percentage > 100) percentage = 100;

    current_config.target_fan_speed = percentage;
    current_config.fan_control_mode = 1;
}

void nvidia_set_clocks(uint32_t core_mhz, uint32_t mem_mhz) {
    if (!nvidia_detected) return;

    if (core_mhz < 100) core_mhz = 100;
    if (core_mhz > 2000) core_mhz = 2000;
    if (mem_mhz < 100) mem_mhz = 100;
    if (mem_mhz > 10000) mem_mhz = 10000;

    current_config.current_core_clock = core_mhz;
    current_config.current_memory_clock = mem_mhz;
}

uint32_t nvidia_get_temperature(void) {
    if (!nvidia_detected) return 0;

    static uint32_t counter = 0;
    uint32_t temp = 45 + (counter % 30);
    counter++;

    gpu_info.temperature = temp;
    return temp;
}

uint32_t nvidia_get_fan_speed(void) {
    if (!nvidia_detected) return 0;

    gpu_info.fan_speed = current_config.target_fan_speed;
    return gpu_info.fan_speed;
}

uint32_t nvidia_get_clock_core(void) {
    if (!nvidia_detected) return 0;

    return current_config.current_core_clock;
}

uint32_t nvidia_get_clock_memory(void) {
    if (!nvidia_detected) return 0;

    return current_config.current_memory_clock;
}
