#include <stdint.h>

int detect_cdrom_boot(uint8_t boot_drive) {
#if defined(__x86__) || defined(x86)

    struct {
        uint8_t size;
        uint8_t media_type;    // media type
        uint8_t drive_number;  // driver numb
        uint8_t controller_id;
        uint32_t lba;          // Start LBA
        uint16_t device_spec;
        uint16_t buffer_ptr;
        uint16_t buffer_seg;
    } __attribute__((packed)) spec_packet;

    spec_packet.size = sizeof(spec_packet);
    uint16_t status = 0;

    __asm__ volatile (
        "int $0x13"
        : "=a"(status)
        : "a"(0x4b01), "d"(boot_drive), "S"(&spec_packet)
        : "memory"
    );

    return ((status & 0xFF) == 0);
#else
    return 0;
#endif
}
