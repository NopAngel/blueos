#include <include/profile.h>
#include <include/colors.h>

unsigned int *prof_buffer;
unsigned int prof_len;
unsigned int prof_start;

void profile_init(unsigned int start_addr, unsigned int end_addr) {
    prof_start = start_addr;
  
    prof_len = (end_addr - start_addr) >> PROF_SHIFT;
    prof_buffer = (unsigned int *)0x200000;

    for (unsigned int i = 0; i < prof_len; i++) {
        prof_buffer[i] = 0;
    }

    printk(GREEN, "\nPROFILER: Initialized for range %x - %x\n", start_addr, end_addr);
}


void profile_tick(unsigned int pc) {
    if (pc >= prof_start) {
        pc -= prof_start;
        pc >>= PROF_SHIFT;

        if (pc < prof_len) {
            prof_buffer[pc]++;
        }
    }
}

void profile_display() {
    printk(YELLOW, "\n--- KERNEL PROFILE STATS ---\n");
    for (unsigned int i = 0; i < prof_len; i++) {
        if (prof_buffer[i] > 0) {
            unsigned int addr = prof_start + (i << PROF_SHIFT);
            printk(WHITE, "Addr %x: %d hits\n", addr, prof_buffer[i]);
        }
    }
}