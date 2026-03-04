#include <kernel/vmcore_info.h>
#include <version.h>
#include <blueos/types.h>
#include <blueos/task.h>
#include <stddef.h>
void dump_vmcoreinfo(void) {
    printk("--- BEGIN VMCOREINFO ---\n");

    printk("VMCOREINFO: OSRELEASE=%s\n", UTS_RELEASE);
    printk("VMCOREINFO: PAGESIZE=%d\n", 4096);

  
    VMCOREINFO_SIZE(task_t);
    VMCOREINFO_OFFSET(task_t, id);
    VMCOREINFO_OFFSET(task_t, state);


    #ifdef CONFIG_MODULES
        VMCOREINFO_CONFIG(MODULES);
    #endif
    #ifdef CONFIG_VFS
        VMCOREINFO_CONFIG(VFS);
    #endif
    VMCOREINFO_OFFSET(task_t, rsp);
    #if defined(__i386__)
        printk("VMCOREINFO: ARCH=i386\n");
    #elif defined(__x86_64__)
        printk("VMCOREINFO: ARCH=x86_64\n");
    #endif

    printk("--- END VMCOREINFO ---\n");
}