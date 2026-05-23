#ifndef EDAC_H
#define EDAC_H

#include <stdint.h>

struct edac_mc_stats {
    uint32_t ce_count; 
    uint32_t ue_count; 
    char controller_name[32];
};

void edac_init();
void edac_check_errors();
void edac_report_ce(const char *msg, uint64_t addr);
void edac_report_ue(const char *msg, uint64_t addr);

#endif