#include <fs/fs.h>
#include <fs/vboxfs.h>
#include <fs/vfs.h>
#include <idt.h>
#include <drivers/scsi.h>

extern fs_ops_t jfs_ops;
extern fs_ops_t xfs_ops;
extern int current_user_index;

void init_all ()
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
    task_init();
    mm_init();
    lru_init();
    profile_init(0x100000, 0x200000);
    pinctrl_init();
    find_wifi_card();
    leds_init();
    vhost_init(); 
    apic_init();

    if (kvm_check_support() == VMX_OK) {
        printk(RED, "KVM: Virtual Machine Extensions ready to be engaged.\n");
    } else {
        printk(RED,"KVM: Error - VT-x not supported or disabled in BIOS.\n");
    }

    kvm_unlock_vmx(); 
    if (kvm_check_support() == VMX_OK) {
        printk(RED,"KVM: Intel VT-x is now ACTIVE!\n");
    }
    __asm__ volatile ("sti");
}
