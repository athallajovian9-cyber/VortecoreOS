// =============================================================================
// VortecoreOS - Linux-Style OS Installer & Partition Manager (installer.h)
// Features: MBR partitioning, formatting VFS disk filesystem, kernel installation
// =============================================================================

#ifndef INSTALLER_H
#define INSTALLER_H

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

// Standard MBR Partition Entry (16 bytes)
typedef struct __attribute__((packed)) {
    uint8_t  bootable;       // 0x80 = Active / Bootable, 0x00 = Inactive
    uint8_t  start_head;
    uint8_t  start_sector;
    uint8_t  start_cylinder;
    uint8_t  sys_id;         // 0x83 = Linux native, 0x07 = NTFS, 0x0B = FAT32, 0x88 = VortecoreFS
    uint8_t  end_head;
    uint8_t  end_sector;
    uint8_t  end_cylinder;
    uint32_t start_lba;      // LBA start sector
    uint32_t total_sectors;  // Partition sector length
} mbr_partition_t;

// Target install drive options
#define INSTALL_TARGET_PRIMARY_DISK 0
#define INSTALL_TARGET_RAMDISK      1

int installer_run(void);

#endif
