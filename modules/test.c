/*
 * modules/test_sandbox.c
 * Hard-coded Direct Pointer Laboratory - Refined
 */

#include <kernel/printk.h>
#include <kernel/process.h>

extern int sys_pledge(const char *promises);
extern bool security_check_syscall(uint32_t syscall_category);

/* Grab the exact global pointer being used by sched.c */
extern task_t *current_task;

int module_init(void) {
    printk("\n\033[36m[SANDBOX TEST]: Running Hard-Coded Execution Test...\033[0m\n");

    /* If the pointer is still NULL, we force it to our emergency structure right here */
    if (!current_task) {
        printk("[SANDBOX TEST]: current_task was NULL in module context. Forcing patch...\n");
        static task_t local_boot_task;
        local_boot_task.pid = 99;
        local_boot_task.state = 1; /* RUNNING / READY */
        
        char *name_ptr = local_boot_task.name;
        *name_ptr++ = 'm'; *name_ptr++ = 'o'; *name_ptr++ = 'd'; *name_ptr++ = 't'; *name_ptr++ = 'e'; *name_ptr++ = 's'; *name_ptr++ = 't'; *name_ptr = '\0';
        
        current_task = &local_boot_task;
    }

    /* Clear any garbage in the sandbox fields */
    current_task->pledge_active = false;
    current_task->pledge_mask = 0;
    current_task->unveil_count = 0;

    printk("[SANDBOX TEST]: Verified Target Context -> PID: %d, Name: %s\n", 
           (int)current_task->pid, current_task->name);

    /* 1. Restrict context to basic IO (stdio) */
    printk("[SANDBOX TEST]: Calling sys_pledge(\"stdio\")...\n");
    sys_pledge("stdio");

    printk("[SANDBOX TEST]: Post-Pledge stats -> Active: %d, Mask: 0x%x\n", 
           (int)current_task->pledge_active, current_task->pledge_mask);

    /* 2. Test allowed operation (stdio) */
    printk("[SANDBOX TEST]: Simulating allowed syscall (STDIO)... ");
    if (security_check_syscall(PLEDGE_STDIO)) {
        printk("\033[32m[PASS]\033[0m\n");
    } else {
        printk("\033[31m[FAIL]: Allowed syscall blocked!\033[0m\n");
        return -1;
    }

    /* 3. Test forbidden operation (rpath) */
    printk("[SANDBOX TEST]: Simulating forbidden syscall (RPATH)... \n");
    
    /* We capture the return in a single call to avoid duplicate logs */
    if (security_check_syscall(PLEDGE_RPATH) == false) {
        /* PASS: The engine returned false, meaning the syscall was safely blocked! */
        printk("\033[32m[PASS]: Sandbox successfully blocked forbidden syscall!\033[0m\n");
        printk("\033[32m[SANDBOX TEST COMPLETE]: ALL TESTS PASSED SUCCESSFULLY.\033[0m\n\n");
        return 0; /* Return success to the kernel load flow */
    } else {
        /* FAIL: If it had returned true, the check would have been bypassed */
        printk("\033[31m[FAIL]: Sandbox bypassed! Unauthorized access allowed.\033[0m\n");
        return -1;
    }
}