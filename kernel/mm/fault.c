#include <asm/ptrace.h>    
#include <kernel/process.h>
#include <kernel/printk.h>
#include <stdint.h>


extern int handle_demand_paging(struct task_struct *task, uintptr_t addr);
extern int is_cow_mapping(uintptr_t addr);
extern void handle_cow_fault(struct task_struct *task, uintptr_t addr);

void do_page_fault(struct pt_regs *regs, uintptr_t stval, uintptr_t scause) {
    struct task_struct *curr = get_current_task();

    printk(RED, "[MMU] Fault at 0x%x | Cause: %d | PC: 0x%x\n", 
           stval, scause, regs->sepc);

    if (stval == 0) {
        printk(RED, "Panic: Null pointer! Killing %s\n", curr->name);
        do_exit(-1);
    }

    if (handle_demand_paging(curr, stval)) {
        return; 
    }

    do_exit(-11);
}