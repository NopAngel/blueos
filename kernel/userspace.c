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
    mm_memset(&tss_entry, 0, sizeof(tss_entry));
    tss_entry.ss0 = ss0;
    tss_entry.esp0 = esp0;
    tss_entry.iomap_base = sizeof(tss_entry);
}


void jump_to_user(void* address) {
    printk(YELLOW, "Namespace: Switching to [%s]\n", user_ns.hostname);
    printk(CYAN, "Userspace: Jumping to Ring 3...\n");


    __asm__z volatile(
        "cli;"
        "mov $0x23, %ax;"   
        "mov %ax, %ds;"
        "mov %ax, %es;"
        "mov %ax, %fs;"
        "mov %ax, %gs;"
        
        "mov %esp, %eax;"
        "pushl $0x23;"     
        "pushl %eax;"          
        "pushf;"         
        "popl %eax;"
        "orl $0x200, %eax;"    
        "pushl %eax;"
        "pushl $0x1B;"        
        "pushl %1;"          
        "iret;"
        : : "r" (address), "m" (address)
    );
}

void print_current_namespace() {
    printk(CYAN, "\n[NS: %s] ", user_ns.hostname);
}


void test_userspace_logic() {

    while(1) {
        asm volatile("nop");
    }
}