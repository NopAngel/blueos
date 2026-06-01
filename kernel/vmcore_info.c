#include <kernel/vmcore_info.h>
#include <kernel/printk.h>
#include <kernel/colors.h>
#include <version.h>
#include <kernel/types.h>
#include <kernel/task.h>
#include <kernel/arch.h>
#include <stddef.h>


void dump_vmcoreinfo(void) {
    printk("--- BEGIN VMCOREINFO ---\n");

    printk("VMCOREINFO: OSRELEASE=%s\n", UTS_RELEASE);

    #ifdef PAGE_SIZE
        printk("VMCOREINFO: PAGESIZE=%d\n", PAGE_SIZE);
    #else
        printk("VMCOREINFO: PAGESIZE=%d\n", 4096); // Fallback
    #endif

    VMCOREINFO_SIZE(task_t);
    VMCOREINFO_OFFSET(task_t, pid);
    VMCOREINFO_OFFSET(task_t, name);

    #ifdef CONFIG_MODULES
        VMCOREINFO_CONFIG(MODULES);
    #endif

    #ifdef CONFIG_VFS
        VMCOREINFO_CONFIG(VFS);
    #endif


    VMCOREINFO_OFFSET(task_t, esp);

    printk("VMCOREINFO: ARCH=%s\n", BLUEOS_ARCH);

    printk("--- END VMCOREINFO ---\n");
}
