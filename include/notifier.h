#ifndef _LINUX_NOTIFIER_H
#define _LINUX_NOTIFIER_H

#include <include/printk.h>

#define NOTIFY_DONE      0x0000   
#define NOTIFY_OK        0x0001   
#define NOTIFY_STOP_MASK 0x8000    
#define NOTIFY_BAD       (NOTIFY_STOP_MASK | 0x0002)

struct notifier_block {
    int (*notifier_call)(struct notifier_block *nb, unsigned long action, void *data);
    struct notifier_block *next;
    int priority; 
};


int notifier_chain_register(struct notifier_block **list, struct notifier_block *nb);
int notifier_chain_unregister(struct notifier_block **list, struct notifier_block *nb);
int notifier_call_chain(struct notifier_block **list, unsigned long action, void *data);

#endif