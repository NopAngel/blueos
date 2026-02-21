#include <include/notifier.h>
#include <stddef.h>


int notifier_chain_register(struct notifier_block **nl, struct notifier_block *n) {
    while ((*nl) != NULL) {
        if (n->priority > (*nl)->priority)
            break;
        nl = &((*nl)->next);
    }
    n->next = *nl;
    *nl = n;
    return 0;
}


int notifier_chain_unregister(struct notifier_block **nl, struct notifier_block *n) {
    while ((*nl) != NULL) {
        if ((*nl) == n) {
            *nl = n->next;
            return 0;
        }
        nl = &((*nl)->next);
    }
    return -1;
}


int notifier_call_chain(struct notifier_block **nl, unsigned long action, void *data) {
    int ret = NOTIFY_DONE;
    struct notifier_block *nb = *nl;

    while (nb) {
        ret = nb->notifier_call(nb, action, data);
        
        if ((ret & NOTIFY_STOP_MASK) == NOTIFY_STOP_MASK) {
            break;
        }
        nb = nb->next;
    }
    return ret;
}