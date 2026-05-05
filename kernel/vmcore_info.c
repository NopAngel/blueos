#include <kernel/vmcore_info.h>
#include <kernel/printk.h>
#include <kernel/colors.h>
#include <version.h>
#include <kernel/types.h>
#include <kernel/task.h>
#include <stddef.h>
void dump_vmcoreinfo(void) {
    printk(WHITE, "--- BEGIN VMCOREINFO ---\n");

    printk(WHITE, "VMCOREINFO: OSRELEASE=%s\n", UTS_RELEASE);
    printk(WHITE, "VMCOREINFO: PAGESIZE=%d\n", 4096);


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
        printk(CYAN, "VMCOREINFO: ARCH=i386\n");
    #elif defined(__x86_64__)
        printk(CYAN, "VMCOREINFO: ARCH=x86_64\n");
    #endif

    printk(WHITE, "--- END VMCOREINFO ---\n");
}
