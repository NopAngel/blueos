#include <drivers/edac.h>
#include <kernel/printk.h>

static struct edac_mc_stats main_mc;

void edac_init() {
    main_mc.ce_count = 0;
    main_mc.ue_count = 0;
    printk(CYAN, "[EDAC] Memory Error Detection and Correction active.\n");
}

void edac_report_ce(const char *msg, uint64_t addr) {
    main_mc.ce_count++;
    printk(YELLOW, "[EDAC] CE (Corrected): %s at 0x%lx (Total: %d)\n", 
           msg, addr, main_mc.ce_count);

}


void edac_report_ue(const char *msg, uint64_t addr) {
    main_mc.ue_count++;
    printk(RED, "!!! [EDAC] UE (Uncorrectable): %s at 0x%lx !!!\n", msg, addr);
    
    // kernel_panic("Memory corruption detected!");
}