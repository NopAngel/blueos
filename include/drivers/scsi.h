#ifndef _BLUEOS_SCSI_H
#define _BLUEOS_SCSI_H

#define SCSI_CMD_TEST_UNIT_READY 0x00
#define SCSI_CMD_INQUIRY 0x12
#define SCSI_CMD_READ_10 0x28
#define SCSI_CMD_WRITE_10 0x2A

typedef struct {
  unsigned char opcode;
  unsigned char lun;
  unsigned int lba;
  unsigned short transfer_length;
  unsigned char control;
} scsi_cdb10_t;

#endif