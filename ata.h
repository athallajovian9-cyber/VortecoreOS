// =============================================================================
// VortecoreOS - ATA PIO Hard Disk Controller Driver (ata.h)
// Architecture: Primary Bus (Port 0x1F0 - 0x1F7), 28-bit / 48-bit LBA Disk I/O
// =============================================================================

#ifndef ATA_H
#define ATA_H

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

// Primary ATA Bus I/O Ports
#define ATA_DATA_PORT       0x1F0
#define ATA_FEATURES_PORT   0x1F1
#define ATA_SECTOR_COUNT    0x1F2
#define ATA_LBA_LOW         0x1F3
#define ATA_LBA_MID         0x1F4
#define ATA_LBA_HIGH        0x1F5
#define ATA_DRIVE_HEAD      0x1F6
#define ATA_COMMAND_STATUS  0x1F7

// ATA Commands
#define ATA_CMD_READ_SECTORS  0x20
#define ATA_CMD_WRITE_SECTORS 0x30
#define ATA_CMD_IDENTIFY      0xEC

// Status Register Bits
#define ATA_STATUS_BSY  0x80 // Busy
#define ATA_STATUS_DRDY 0x40 // Drive Ready
#define ATA_STATUS_DF   0x20 // Drive Fault
#define ATA_STATUS_DRQ  0x08 // Data Request ready
#define ATA_STATUS_ERR  0x01 // Error

int ata_init(void);
int ata_read_sector(uint32_t lba, uint8_t* buffer);
int ata_write_sector(uint32_t lba, const uint8_t* buffer);

#endif
