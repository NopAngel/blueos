#define BT_HCI_CMD 0x01
#define BT_HCI_ACL 0x02
#define BT_HCI_SCO 0x03
#define BT_HCI_EVT 0x04

#define BT_HCI_RESET 0x0C03
#define BT_HCI_READ_BD_ADDR 0x1009
#define BT_HCI_SET_EVENT_MASK 0x0C01
#define BT_HCI_WRITE_SCAN_ENABLE 0x0C1A

struct bt_hci_cmd_hdr {
    unsigned short opcode;
    unsigned char plen;
};

struct bt_hci_evt_hdr {
    unsigned char evt;
    unsigned char plen;
};

struct bt_hci_acl_hdr {
    unsigned short handle;
    unsigned short dlen;
};

unsigned char bt_send_cmd(unsigned short opcode, unsigned char plen, void *param);
void bt_send_acl(unsigned short handle, unsigned short len, void *data);
void bt_process_packet(unsigned char type, void *data, unsigned short len);

void bt_reset(void) {
    bt_send_cmd(BT_HCI_RESET, 0, 0);
}

void bt_set_event_mask(void) {
    unsigned long long mask = 0xFFFFFFFFFFFFFFFFULL;
    bt_send_cmd(BT_HCI_SET_EVENT_MASK, 8, &mask);
}

void bt_read_bd_addr(void) {
    bt_send_cmd(BT_HCI_READ_BD_ADDR, 0, 0);
}

void bt_write_scan_enable(unsigned char enable) {
    bt_send_cmd(BT_HCI_WRITE_SCAN_ENABLE, 1, &enable);
}

void bt_init(void) {
    bt_reset();
    for(volatile int i=0;i<10000;i++);
    bt_set_event_mask();
    bt_read_bd_addr();
    bt_write_scan_enable(0x03);
}

unsigned char bt_process_event(unsigned char *data, unsigned short len) {
    struct bt_hci_evt_hdr *hdr = (struct bt_hci_evt_hdr*)data;
    unsigned char *param = data + sizeof(struct bt_hci_evt_hdr);

    if(hdr->evt == 0x0E) {
        unsigned char num = param[0];
        unsigned char *ptr = param + 1;
        for(int i=0;i<num;i++) {
            unsigned short opcode = ptr[0] | (ptr[1] << 8);
            unsigned char status = ptr[2];
            ptr += 3;

            if(opcode == BT_HCI_RESET) {
                if(status == 0) {
                }
            }
        }
    }
    else if(hdr->evt == 0x0F) {
        unsigned short opcode = param[0] | (param[1] << 8);
        unsigned char status = param[2];

        if(opcode == BT_HCI_READ_BD_ADDR) {
            unsigned char *addr = param + 3;
        }
    }

    return 0;
}

void bt_rx_handler(unsigned char *data, unsigned short len) {
    unsigned char type = data[0];
    unsigned short payload_len = 0;

    switch(type) {
        case BT_HCI_EVT:
            if(len >= 2) {
                struct bt_hci_evt_hdr *hdr = (struct bt_hci_evt_hdr*)(data+1);
                payload_len = hdr->plen + 2;
                if(len >= payload_len) {
                    bt_process_event(data+1, payload_len);
                }
            }
            break;

        case BT_HCI_ACL:
            if(len >= 4) {
                struct bt_hci_acl_hdr *hdr = (struct bt_hci_acl_hdr*)(data+1);
                payload_len = (hdr->dlen & 0xFFF) + 4;
                if(len >= payload_len) {
                    unsigned char *acl_data = data + 5;
                    unsigned short acl_len = payload_len - 4;
                }
            }
            break;
    }
}

struct l2cap_hdr {
    unsigned short len;
    unsigned short cid;
};

struct l2cap_cmd_hdr {
    unsigned char code;
    unsigned char id;
    unsigned short len;
};

#define L2CAP_CID_SIGNALING 0x0001
#define L2CAP_CID_CONN_REQ 0x02
#define L2CAP_CID_CONN_RSP 0x03
#define L2CAP_CID_CFG_REQ 0x04
#define L2CAP_CID_CFG_RSP 0x05

void l2cap_send_signaling(unsigned short handle, unsigned char code, unsigned char id, unsigned short len, void *data) {
    unsigned char packet[256];
    struct l2cap_hdr *l2hdr = (struct l2cap_hdr*)packet;
    struct l2cap_cmd_hdr *cmdhdr = (struct l2cap_cmd_hdr*)(packet + sizeof(struct l2cap_hdr));

    l2hdr->len = len + sizeof(struct l2cap_cmd_hdr);
    l2hdr->cid = L2CAP_CID_SIGNALING;
    cmdhdr->code = code;
    cmdhdr->id = id;
    cmdhdr->len = len;

    unsigned char *ptr = packet + sizeof(struct l2cap_hdr) + sizeof(struct l2cap_cmd_hdr);
    for(int i=0;i<len;i++) ptr[i] = ((unsigned char*)data)[i];

    unsigned short total = sizeof(struct l2cap_hdr) + sizeof(struct l2cap_cmd_hdr) + len;
    bt_send_acl(handle, total, packet);
}

void l2cap_process_signaling(unsigned short handle, unsigned char *data, unsigned short len) {
    struct l2cap_cmd_hdr *hdr = (struct l2cap_cmd_hdr*)data;
    unsigned char *param = data + sizeof(struct l2cap_cmd_hdr);

    switch(hdr->code) {
        case L2CAP_CID_CONN_REQ:
            {
                unsigned short psm = param[0] | (param[1] << 8);
                unsigned short scid = param[2] | (param[3] << 8);
                unsigned char rsp[8];
                rsp[0] = 0x01; rsp[1] = 0x00;
                rsp[2] = scid & 0xFF; rsp[3] = (scid >> 8) & 0xFF;
                rsp[4] = 0x40; rsp[5] = 0x00;
                rsp[6] = 0x00; rsp[7] = 0x00;
                l2cap_send_signaling(handle, L2CAP_CID_CONN_RSP, hdr->id, 8, rsp);
            }
            break;
    }
}

#define SDP_PSM 0x0001
#define RFCOMM_PSM 0x0003

struct rfcomm_hdr {
    unsigned char addr;
    unsigned char ctrl;
    unsigned char len;
};

void rfcomm_send_sabm(unsigned char dlci) {
    struct rfcomm_hdr hdr;
    hdr.addr = (dlci << 2) | 0x01;
    hdr.ctrl = 0x2F;
    hdr.len = 0x00;
}

#define BT_CLASS_COMPUTER 0x0100
#define BT_CLASS_PHONE 0x0200

void bt_set_class(unsigned int class) {
    unsigned char cmd[3];
    cmd[0] = class & 0xFF;
    cmd[1] = (class >> 8) & 0xFF;
    cmd[2] = (class >> 16) & 0xFF;
    bt_send_cmd(0x0C24, 3, cmd);
}

void bt_start_inquiry(void) {
    unsigned char cmd[5];
    cmd[0] = 0x33;
    cmd[1] = 0x8B;
    cmd[2] = 0x9E;
    cmd[3] = 0x08;
    cmd[4] = 0x00;
    bt_send_cmd(0x0401, 5, cmd);
}

void bt_enable_paging(void) {
    unsigned char cmd[1];
    cmd[0] = 0x01;
    bt_send_cmd(0x0419, 1, cmd);
}
