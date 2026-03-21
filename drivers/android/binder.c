#include <stdint.h>
#include <kernel/process.h>
#include <kernel/ioctl.h>
#include <lib/string.h>

#define BINDER_WRITE_READ _IOWR('b', 1, struct binder_write_read)

struct binder_write_read {
    uintptr_t write_buffer;
    uintptr_t read_buffer;
    uint32_t  write_size;
    uint32_t  read_size;
};


long android_binder_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
    struct task_struct *curr = get_current_task();
    
    switch (cmd) {
        case BINDER_WRITE_READ:
            struct binder_write_read bwr;
            memcpy(&bwr, (void*)arg, sizeof(bwr));

            if (bwr.write_size > 0) {
                return binder_transaction(curr, bwr.write_buffer, bwr.write_size);
            }
            break;

        default:
            return -1;
    }
    return 0;
}


int binder_transaction(struct task_struct *src, uintptr_t data, uint32_t size) {
    return 0; 
}