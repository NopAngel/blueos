#include "../../include/drivers/nvidia/core.h"
#include "../../include/types.h"

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

int nvidia_mgmt_detect(void) {
    gpu_info.vendor_id = 0x10DE;
    gpu_info.device_id = 0x0194;
    gpu_info.vram_size = 1024;
    gpu_info.core_clock = 700;
    gpu_info.memory_clock = 924;
    
    simple_strcpy(gpu_info.device_name, "NVIDIA GeForce GTX 480");
    simple_strcpy(gpu_info.family_name, "GeForce GTX 400");
    
    current_config.current_core_clock = gpu_info.core_clock;
    current_config.current_memory_clock = gpu_info.memory_clock;
    current_config.target_fan_speed = 40;
    current_config.fan_control_mode = 0;
    
    nvidia_detected = 1;
    
    return 1;
}

void nvidia_mgmt_init(void) {
    nvidia_mgmt_detect();
}

void nvidia_print_info(void) {
    if (!nvidia_detected) return;
    
    char buffer[80];
    
    simple_strcpy(buffer, "Device: ");
    simple_strcat(buffer, gpu_info.device_name);
    
    simple_strcpy(buffer, "VRAM: ");
    char temp_str[20];
    uint32_t temp = gpu_info.vram_size;
    int idx = 0;
    if (temp == 0) {
        temp_str[idx++] = '0';
    } else {
        while (temp > 0) {
            temp_str[idx++] = '0' + (temp % 10);
            temp /= 10;
        }
        for (int i = 0; i < idx / 2; i++) {
            char t = temp_str[i];
            temp_str[i] = temp_str[idx - 1 - i];
            temp_str[idx - 1 - i] = t;
        }
    }
    temp_str[idx] = '\0';
    simple_strcat(buffer, temp_str);
    simple_strcat(buffer, " MB");
    
    simple_strcpy(buffer, "Core: ");
    idx = 6;
    temp = gpu_info.core_clock;
    if (temp == 0) {
        buffer[idx++] = '0';
    } else {
        while (temp > 0) {
            buffer[idx++] = '0' + (temp % 10);
            temp /= 10;
        }
        for (int i = 6; i < idx / 2 + 6; i++) {
            char t = buffer[i];
            buffer[i] = buffer[idx - 1 - (i - 6)];
            buffer[idx - 1 - (i - 6)] = t;
        }
    }
    simple_strcat(buffer, " MHz");
    
    simple_strcpy(buffer, "Memory: ");
    idx = 8;
    temp = gpu_info.memory_clock;
    if (temp == 0) {
        buffer[idx++] = '0';
    } else {
        while (temp > 0) {
            buffer[idx++] = '0' + (temp % 10);
            temp /= 10;
        }
        for (int i = 8; i < idx / 2 + 8; i++) {
            char t = buffer[i];
            buffer[i] = buffer[idx - 1 - (i - 8)];
            buffer[idx - 1 - (i - 8)] = t;
        }
    }
    simple_strcat(buffer, " MHz");
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