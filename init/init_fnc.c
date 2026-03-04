#include <fs/fs.h>
#include <fs/vboxfs.h>
#include <fs/vfs.h>
#include <idt.h>
#include <hlec.h>
#include <drivers/scsi.h>
#include <blueos/kvm.h>
#include <multiboot.h>
#include <hpet.h>

extern uint32_t _end; 
extern uint32_t used_blocks;
extern uint32_t total_blocks;
extern fs_ops_t jfs_ops;
extern fs_ops_t xfs_ops;
extern int current_user_index;
void pmm_init(uint32_t mem_size, uint32_t bitmap_addr);
void pmm_init_region(uint32_t base, uint32_t size);
void pmm_set_bit(uint32_t bit);
int  pmm_test_bit(uint32_t bit);

hpet_table_t* find_hpet_table(multiboot_info_t* mbi) {

    char* rsdp_search = (char*)0x000E0000;
    uint32_t rsdp_addr = 0;

    for (uint32_t i = 0; i < 0x20000; i += 16) {
        if (*(uint64_t*)(rsdp_search + i) == 0x2052545020445352) { 
            rsdp_addr = (uint32_t)(rsdp_search + i);
            break;
        }
    }

    if (!rsdp_addr) return 0;


    uint32_t rsdt_addr = *(uint32_t*)(rsdp_addr + 16);
    struct acpi_sdt_header* rsdt = (struct acpi_sdt_header*)rsdt_addr;

    uint32_t entries = (rsdt->length - sizeof(struct acpi_sdt_header)) / 4;
    uint32_t* table_ptr = (uint32_t*)(rsdt_addr + sizeof(struct acpi_sdt_header));

    for (uint32_t i = 0; i < entries; i++) {
        struct acpi_sdt_header* h = (struct acpi_sdt_header*)table_ptr[i];
        if (h->signature[0] == 'H' && h->signature[1] == 'P' && 
            h->signature[2] == 'E' && h->signature[3] == 'T') {
            return (hpet_table_t*)h;
        }
    }
    return 0;
}


void init_all (unsigned int magic, struct multiboot_info *mbd)
{
    clear_screen();

    


    vfs_mkdir("/base");
    vfs_mkdir("/base/inf");

    char *help_content = 
        "--- BlueOS Help System ---\n"
        "Available commands:\n"
        " - bluefetch : Show system info and the raccoon.\n"
        " - ls        : List files in current directory.\n"
        " - cat <file>: Read file content.\n"
        " - login     : Authenticate user.\n"
        " - clear     : Wipe the terminal screen.\n"
        " - help      : Show this manual.\n"
        "--------------------------\n";
    idt_init();

    vfs_create("/base/inf/info.bluehelp", help_content);
    sysfs_init();
    fs_init();
    jfs_init();
    vfs_init();
    
    scsi_init();
    auth_init();
    current_user_index = -1; 
    tty_init();

    int virt_ok = 0;
    virt_ok = init_intel_vtx();
    if (!virt_ok) {
        virt_ok = init_amd_svm();
    }


    if (virt_ok) {
        printk(GREEN, "BlueOS: Loaded hardware virtualization engine.\n");

    } else {
        printk(YELLOW,"BlueOS: Running in legacy mode (No VM support).\n");
    }

    mm_init();

    hpet_table_t* hpet_tab = find_hpet_table(mbd);

    if (hpet_tab) 
    {
        hpet_init(hpet_tab);
        printk(GREEN, "\nHPET found and initialized successfully.\n");
    } 
    else 
    {
        printk(RED, "\nHPET table not found. Timer functionality may be limited.\n");
    }

    lru_init();
    profile_init(0x100000, 0x200000);
    pinctrl_init();
    find_wifi_card();
    leds_init();
    vhost_init(); 
    apic_init();

    /*if (kvm_check_support() == VMX_OK) {
        printk(RED, "KVM: Virtual Machine Extensions ready to be engaged.\n");
    } else {
        printk(RED,"KVM: Error - VT-x not supported or disabled in BIOS.\n");
    }

    kvm_unlock_vmx(); 
    if (kvm_check_support() == VMX_OK) {
        printk(RED,"KVM: Intel VT-x is now ACTIVE!\n");
    }*/


    uint32_t k_start = 0x100000;
    uint32_t k_end = (uint32_t)&_end;

    for (uint32_t addr = k_start; addr < k_end; addr += 4096) {
        if (!pmm_test_bit(addr / 4096)) {
            pmm_set_bit(addr / 4096);
            used_blocks++;
        }
    }

    uint32_t free_blocks = total_blocks - used_blocks;
    printk(YELLOW, "PMM: RAM detected. Free blocks: %d (%d MB)\n", 
           free_blocks, (free_blocks * 4096) / (1024 * 1024));

    __asm__ volatile ("sti");
}
