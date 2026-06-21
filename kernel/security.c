/*
 * kernel/security.c
 * Fixed OpenBSD-style Pledge & Unveil Sandbox Security Engine for BlueOS
 * Pure C, strictly in English.
 */

#include <kernel/printk.h>
#include <kernel/process.h>
#include <stddef.h>
#include <lib/string.h>

/* Grab the exact same global pointer that sched.c and your test module are using */
extern task_t *current_task;

/**
 * sys_pledge - Restricts the current process syscall access capability categories.
 */
int sys_pledge(const char *promises) {
    task_t *curr = current_task; /* Use the direct master global pointer */
    if (!curr) return -1;

    if (curr->pledge_active && curr->pledge_mask == 0 && promises != NULL) {
        return -1; 
    }

    uint32_t new_mask = 0;

    if (promises != NULL) {
        if (strstr(promises, "stdio"))  new_mask |= PLEDGE_STDIO;
        if (strstr(promises, "rpath"))  new_mask |= PLEDGE_RPATH;
        if (strstr(promises, "wpath"))  new_mask |= PLEDGE_WPATH;
        if (strstr(promises, "cpath"))  new_mask |= PLEDGE_CPATH;
        if (strstr(promises, "drv"))    new_mask |= PLEDGE_DRV;
        
        if (curr->pledge_active && (new_mask & ~curr->pledge_mask) != 0) {
            printk("<3> SECURITY VIOLATION: Privileges escalation blocked!\n");
            return -1;
        }
    }

    curr->pledge_mask = new_mask;
    curr->pledge_active = true; /* THIS WILL NOW SUCCESSFULLY FLIP TO TRUE (1) */
    
    printk("<6> SECURITY: Sandboxed process [%s] (PID:%d) with mask 0x%x\n", curr->name, curr->pid, new_mask);
    return 0;
}

/**
 * security_check_syscall - Interceptor called by low-level kernel routines
 */
bool security_check_syscall(uint32_t syscall_category) {
    task_t *curr = current_task;
    
    if (!curr || !curr->pledge_active) return true;

    /* Validate explicit binary bitmask authorization */
    if (!(curr->pledge_mask & syscall_category)) {
        printk("\n\033[31m<3> PLEDGE VIOLATION: Process [%s] (PID:%d) attempted unauthorized syscall 0x%x\033[0m\n", 
               curr->name, curr->pid, syscall_category);
        
        /* * If it's the early boot kernel task, we don't want to freeze the whole machine!
         * We just mark the context and allow the system to keep booting.
         */
        if (curr->pid == 0) {
            printk("\033[33m[SECURITY]: Early Boot Context (PID 0). Skipping hard hardware freeze to prevent kernel lockup.\033[0m\n");
            return false; /* Deny the syscall but keep the kernel spinning */
        }

        /* For future user-space apps, we terminate them normally */
        do_exit(-1); 
        return false;
    }

    return true;
}