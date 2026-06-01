#include <stdint.h>
#include <lib/string.h>
#include <kernel/printk.h>
#include <kernel/colors.h>


#define MAX_NS_NAME 32

typedef struct {
    int id;
    char hostname[MAX_NS_NAME];
    uint32_t mount_root;
} namespace_t;

static namespace_t kernel_ns = {0, "blueos-kernel", 0};
static namespace_t user_ns   = {1, "blueos-user", 0};


struct tss_entry_struct {
    uint32_t prev_tss;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1, ss1, esp2, ss2, cr3, eip, eflags;
    uint32_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
    uint32_t es, cs, ss, ds, fs, gs, ldt;
    uint16_t trap, iomap_base;
} __attribute__((packed));

static struct tss_entry_struct tss_entry;

void init_tss(uint16_t ss0, uint32_t esp0) {
    extern void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);

    memset(&tss_entry, 0, sizeof(tss_entry));
    tss_entry.ss0 = ss0;
    tss_entry.esp0 = esp0;
    tss_entry.iomap_base = sizeof(tss_entry);

    uint32_t base = (uint32_t)&tss_entry;
    gdt_set_gate(5, base, sizeof(tss_entry) - 1, 0x89, 0x00);

    __asm__ volatile("ltr %%ax" : : "a" (0x28));
}


void jump_to_user(void* address) {
    extern void asm_jump_to_user(void* address);
    printk("Namespace: Switching to [%s]\n", user_ns.hostname);
    printk("Userspace: Jumping to Ring 3 via Assembly...\n");
    asm_jump_to_user(address);
}

void print_current_namespace() {
    printk("\n[NS: %s] ", user_ns.hostname);
}


void test_userspace_logic() {

    while(1) {
        asm volatile("nop");
    }
}
