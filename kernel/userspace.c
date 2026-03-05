#include <stdint.h>
#include <blueos/printk.h>
#include <blueos/colors.h>

struct tss_entry_struct {
    uint32_t prev_tss;   
    uint32_t esp0;      
    uint32_t ss0;        
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax, ecx, edx, ebx;
    uint32_t esp, ebp, esi, edi;
    uint32_t es, cs, ss, ds, fs, gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed));

typedef struct tss_entry_struct tss_entry_t;

static tss_entry_t tss_entry;

void write_tss(int32_t num, uint16_t ss0, uint32_t esp0) {


    mm_memset(&tss_entry, 0, sizeof(tss_entry));

    tss_entry.ss0  = ss0;     
    tss_entry.esp0 = esp0;    
    
    tss_entry.iomap_base = sizeof(tss_entry);
}

void set_kernel_stack(uint32_t stack) {
    tss_entry.esp0 = stack;
}


void jump_to_user(void* address) {
    printk(YELLOW, "Userspace: Preparing IRET frame for Ring 3 jump...\n");


    volatile __asm__ (
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
