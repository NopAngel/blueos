#ifndef SOC_INTEL_H
#define SOC_INTEL_H

typedef struct {
    char vendor[13];
    uint32_t family;
    uint32_t model;
    char brand_string[48];
} soc_info_t;




#endif