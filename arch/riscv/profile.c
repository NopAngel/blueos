#include <profile.h>
#include <kernel/printk.h>
#include <kernel/colors.h>
#include <stdint.h>

uintptr_t *prof_buffer;
uintptr_t prof_len;
uintptr_t prof_start;


void profile_init(uintptr_t start_addr, uintptr_t end_addr) {
    prof_start = start_addr;
  
    prof_len = (end_addr - start_addr) >> PROF_SHIFT;

    prof_buffer = (uintptr_t *)0x80400000; 

    for (uintptr_t i = 0; i < prof_len; i++) {
        prof_buffer[i] = 0;
    }

    printk(GREEN, "PROFILER: Initialized for range 0x%lx - 0x%lx\n", start_addr, end_addr);
    printk(GRAY, "PROFILER: Buffer allocated at 0x%lx (%d slots)\n", (uintptr_t)prof_buffer, prof_len);
}

void profile_tick(uintptr_t pc) {
    if (pc >= prof_start) {
        uintptr_t offset = pc - prof_start;
        offset >>= PROF_SHIFT;

        if (offset < prof_len) {
            prof_buffer[offset]++;
        }
    }
}


void profile_display() {
    printk(YELLOW, "\n--- KERNEL PROFILE STATS (RISC-V) ---\n");
    int found_hits = 0;

    for (uintptr_t i = 0; i < prof_len; i++) {
        if (prof_buffer[i] > 0) {
            uintptr_t addr = prof_start + (i << PROF_SHIFT);
            printk(WHITE, " Addr 0x%lx: %d hits\n", addr, prof_buffer[i]);
            found_hits++;
        }
    }

    if (found_hits == 0) {
        printk(GRAY, " No hits recorded yet.\n");
    }
}