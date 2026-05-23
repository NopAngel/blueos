#ifndef MAILBOX_H
#define MAILBOX_H

#include <stdint.h>

#define MBOX_BASE        0x2000B000
#define MBOX_REG_STATUS  (MBOX_BASE + 0x00)
#define MBOX_REG_DATA    (MBOX_BASE + 0x04)
#define MBOX_REG_SENDER  (MBOX_BASE + 0x08)

#define MBOX_FULL        (1 << 0)
#define MBOX_EMPTY       (1 << 1)

struct mbox_msg {
    uint32_t channel;
    uint32_t data;
};

void mbox_init();
int  mbox_send(struct mbox_msg *msg);
int  mbox_receive(struct mbox_msg *msg);

#endif