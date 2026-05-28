#include <stdint.h>
#include <kernel/printk.h>

#define PAGE_SIZE 4096
#define PTE_V     (1 << 0) // Valid
#define PTE_R     (1 << 1) // Read
#define PTE_W     (1 << 2) // Write
#define PTE_X     (1 << 3) // Execute
#define PTE_U     (1 << 4) // User

void print_pte(uintptr_t va, uint64_t pte, int level) {
    uintptr_t pa = (pte >> 10) << 12;
    char r = (pte & PTE_R) ? 'R' : '-';
    char w = (pte & PTE_W) ? 'W' : '-';
    char x = (pte & PTE_X) ? 'X' : '-';
    char u = (pte & PTE_U) ? 'U' : 'S';

    printk(WHITE, "0x%08x | 0x%08x | %c%c%c %c (L%d)\n",
           va, pa, r, w, x, u, level);
}


void ptdump(uintptr_t pgdir) {
    uint64_t *l2 = (uint64_t *)pgdir;

    printk(YELLOW, "--- [ BlueOS Page Table Dump ] ---\n");
    printk(YELLOW, "VA Range           | PA               | Flags\n");

    for (int i = 0; i < 512; i++) {
        if (!(l2[i] & PTE_V)) continue;

        if (l2[i] & (PTE_R | PTE_W | PTE_X)) {
            print_pte(i << 30, l2[i], 2);
            continue;
        }

        uint64_t *l1 = (uint64_t *)((l2[i] >> 10) << 12);
        for (int j = 0; j < 512; j++) {
            if (!(l1[j] & PTE_V)) continue;

            if (l1[j] & (PTE_R | PTE_W | PTE_X)) {
                print_pte((i << 30) | (j << 21), l1[j], 1);
                continue;
            }

            uint64_t *l0 = (uint64_t *)((l1[j] >> 10) << 12);
            for (int k = 0; k < 512; k++) {
                if (l0[k] & PTE_V) {
                    uintptr_t va = ((uintptr_t)i << 30) | ((uintptr_t)j << 21) | (k << 12);
                    print_pte(va, l0[k], 0);
                }
            }
        }
    }
}
