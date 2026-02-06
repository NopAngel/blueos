// src/drivers/net/e1000.c
#define E1000_REG_CTRL 0x0000
#define E1000_REG_EEPROM 0x0014

uint32_t e1000_read_command(uint16_t address) {

}

void e1000_init() {
   
}

void e1000_handle_interrupt() {
  
    while() {
        ethernet_frame_t* frame = (ethernet_frame_t*)packet_data;
        
        if (frame->type == 0x0608) {
            handle_arp(frame);
        } else if (frame->type == 0x0008) {
            handle_ipv4(frame);
        }
    }
}