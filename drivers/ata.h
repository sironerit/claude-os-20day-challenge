#ifndef ATA_H
#define ATA_H

#include "../kernel/types.h"

// ATA/IDE Port Addresses
#define ATA_PRIMARY_BASE    0x1F0
#define ATA_SECONDARY_BASE  0x170

// ATA/IDE Registers (relative to base)
#define ATA_REG_DATA        0x00
#define ATA_REG_ERROR       0x01
#define ATA_REG_FEATURES    0x01
#define ATA_REG_SECCOUNT    0x02
#define ATA_REG_LBA_LOW     0x03
#define ATA_REG_LBA_MID     0x04
#define ATA_REG_LBA_HIGH    0x05
#define ATA_REG_DRIVE       0x06
#define ATA_REG_STATUS      0x07
#define ATA_REG_COMMAND     0x07

// ATA/IDE Control Registers
#define ATA_REG_CONTROL     0x206
#define ATA_REG_ALTSTATUS   0x206

// ATA Status Register Bits
#define ATA_STATUS_BSY      (1 << 7)  // Busy
#define ATA_STATUS_DRDY     (1 << 6)  // Drive Ready
#define ATA_STATUS_DF       (1 << 5)  // Drive Fault
#define ATA_STATUS_DSC      (1 << 4)  // Drive Seek Complete
#define ATA_STATUS_DRQ      (1 << 3)  // Data Request
#define ATA_STATUS_CORR     (1 << 2)  // Corrected Data
#define ATA_STATUS_IDX      (1 << 1)  // Index
#define ATA_STATUS_ERR      (1 << 0)  // Error

// ATA Commands
#define ATA_CMD_IDENTIFY    0xEC
#define ATA_CMD_READ_SECTORS 0x20
#define ATA_CMD_WRITE_SECTORS 0x30

// Drive selection bits
#define ATA_DRIVE_MASTER    0xA0
#define ATA_DRIVE_SLAVE     0xB0

// Drive information structure
typedef struct {
    uint8_t exists;         // 1 if drive exists, 0 otherwise
    uint8_t is_master;      // 1 if master, 0 if slave
    uint16_t base_port;     // Base I/O port
    uint32_t sectors;       // Total number of sectors
    char model[41];         // Drive model string (40 chars + null)
    char serial[21];        // Drive serial number (20 chars + null)
} ata_drive_t;

// Function declarations
void ata_init(void);
int ata_detect_drives(void);
void ata_print_drive_info(void);

// Safe read-only functions for initial implementation
uint8_t ata_read_status(uint16_t base);
void ata_wait_ready(uint16_t base);
int ata_identify_drive(uint16_t base, uint8_t drive, uint16_t* buffer);

// Phase 2: Safe disk reading functions
int ata_read_sectors(uint8_t drive_num, uint32_t lba, uint8_t sector_count, uint16_t* buffer);
int ata_get_drive_info(uint8_t drive_num, ata_drive_t* info);
int ata_test_read(void);

// Phase 3: Safe disk writing functions
int ata_write_sectors(uint8_t drive_num, uint32_t lba, uint8_t sector_count, const uint16_t* buffer);
int ata_test_write(void);
int ata_format_drive(uint8_t drive_num);  // Safe formatting function

#endif