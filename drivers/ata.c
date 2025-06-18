// ATA/IDE Driver - Day 10 Phase 1: Safe Detection and Identification
// Read-only implementation for maximum safety

#include "ata.h"
#include "../kernel/pic.h"      // For I/O functions (inb, outb, inw)
#include "../kernel/kernel.h"
#include "../kernel/string.h"
#include "../kernel/heap.h"     // For kmalloc, kfree

// Global drive array
static ata_drive_t drives[4];  // Primary Master/Slave, Secondary Master/Slave
static int drive_count = 0;

// Initialize ATA subsystem
void ata_init(void) {
    terminal_writestring("ATA: Initializing ATA/IDE subsystem...\n");
    
    // Clear drive array
    for (int i = 0; i < 4; i++) {
        drives[i].exists = 0;
        drives[i].is_master = 0;
        drives[i].base_port = 0;
        drives[i].sectors = 0;
        drives[i].model[0] = '\0';
        drives[i].serial[0] = '\0';
    }
    
    drive_count = 0;
    
    // Detect drives on both channels
    int detected = ata_detect_drives();
    
    if (detected > 0) {
        terminal_writestring("ATA: Drive detection completed successfully\n");
        ata_print_drive_info();
    } else {
        terminal_writestring("ATA: No drives detected\n");
    }
}

// Read ATA status register safely
uint8_t ata_read_status(uint16_t base) {
    return inb(base + ATA_REG_STATUS);
}

// Wait for drive to be ready (with timeout for safety)
void ata_wait_ready(uint16_t base) {
    // Wait up to ~1 second for drive to be ready
    for (int timeout = 0; timeout < 100000; timeout++) {
        uint8_t status = ata_read_status(base);
        
        // If not busy and drive is ready, we're good
        if (!(status & ATA_STATUS_BSY) && (status & ATA_STATUS_DRDY)) {
            return;
        }
        
        // Small delay to prevent CPU spinning
        for (volatile int i = 0; i < 1000; i++);
    }
    
    // Timeout occurred - this is expected if no drive is present
}

// Safely identify a drive (read-only operation)
int ata_identify_drive(uint16_t base, uint8_t drive, uint16_t* buffer) {
    // Select drive
    outb(base + ATA_REG_DRIVE, drive);
    
    // Wait for drive selection to take effect
    ata_wait_ready(base);
    
    // Send IDENTIFY command
    outb(base + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
    
    // Check if drive exists by reading status
    uint8_t status = ata_read_status(base);
    if (status == 0) {
        // No drive present
        return 0;
    }
    
    // Wait for drive to be ready for data transfer
    for (int timeout = 0; timeout < 100000; timeout++) {
        status = ata_read_status(base);
        
        if (status & ATA_STATUS_ERR) {
            // Error occurred - drive might not support IDENTIFY
            return 0;
        }
        
        if (!(status & ATA_STATUS_BSY) && (status & ATA_STATUS_DRQ)) {
            // Drive is ready to send data
            break;
        }
        
        // Small delay
        for (volatile int i = 0; i < 1000; i++);
    }
    
    // Read 256 words (512 bytes) of identify data
    for (int i = 0; i < 256; i++) {
        buffer[i] = inw(base + ATA_REG_DATA);
    }
    
    return 1;  // Success
}

// Extract string from identify data (ATA strings are word-swapped)
static void ata_extract_string(uint16_t* source, char* dest, int start_word, int length) {
    for (int i = 0; i < length / 2; i++) {
        uint16_t word = source[start_word + i];
        dest[i * 2] = (word >> 8) & 0xFF;
        dest[i * 2 + 1] = word & 0xFF;
    }
    dest[length] = '\0';
    
    // Trim trailing spaces
    for (int i = length - 1; i >= 0 && dest[i] == ' '; i--) {
        dest[i] = '\0';
    }
}

// Detect all ATA drives
int ata_detect_drives(void) {
    uint16_t identify_buffer[256];
    int detected = 0;
    
    // Check primary channel
    terminal_writestring("ATA: Checking primary channel (0x1F0)...\n");
    
    // Primary Master
    if (ata_identify_drive(ATA_PRIMARY_BASE, ATA_DRIVE_MASTER, identify_buffer)) {
        drives[detected].exists = 1;
        drives[detected].is_master = 1;
        drives[detected].base_port = ATA_PRIMARY_BASE;
        drives[detected].sectors = ((uint32_t)identify_buffer[61] << 16) | identify_buffer[60];
        
        // Extract model and serial
        ata_extract_string(identify_buffer, drives[detected].model, 27, 40);
        ata_extract_string(identify_buffer, drives[detected].serial, 10, 20);
        
        terminal_writestring("ATA: Primary Master detected\n");
        detected++;
    }
    
    // Primary Slave
    if (ata_identify_drive(ATA_PRIMARY_BASE, ATA_DRIVE_SLAVE, identify_buffer)) {
        drives[detected].exists = 1;
        drives[detected].is_master = 0;
        drives[detected].base_port = ATA_PRIMARY_BASE;
        drives[detected].sectors = ((uint32_t)identify_buffer[61] << 16) | identify_buffer[60];
        
        // Extract model and serial
        ata_extract_string(identify_buffer, drives[detected].model, 27, 40);
        ata_extract_string(identify_buffer, drives[detected].serial, 10, 20);
        
        terminal_writestring("ATA: Primary Slave detected\n");
        detected++;
    }
    
    // Check secondary channel
    terminal_writestring("ATA: Checking secondary channel (0x170)...\n");
    
    // Secondary Master
    if (ata_identify_drive(ATA_SECONDARY_BASE, ATA_DRIVE_MASTER, identify_buffer)) {
        drives[detected].exists = 1;
        drives[detected].is_master = 1;
        drives[detected].base_port = ATA_SECONDARY_BASE;
        drives[detected].sectors = ((uint32_t)identify_buffer[61] << 16) | identify_buffer[60];
        
        // Extract model and serial
        ata_extract_string(identify_buffer, drives[detected].model, 27, 40);
        ata_extract_string(identify_buffer, drives[detected].serial, 10, 20);
        
        terminal_writestring("ATA: Secondary Master detected\n");
        detected++;
    }
    
    // Secondary Slave
    if (ata_identify_drive(ATA_SECONDARY_BASE, ATA_DRIVE_SLAVE, identify_buffer)) {
        drives[detected].exists = 1;
        drives[detected].is_master = 0;
        drives[detected].base_port = ATA_SECONDARY_BASE;
        drives[detected].sectors = ((uint32_t)identify_buffer[61] << 16) | identify_buffer[60];
        
        // Extract model and serial
        ata_extract_string(identify_buffer, drives[detected].model, 27, 40);
        ata_extract_string(identify_buffer, drives[detected].serial, 10, 20);
        
        terminal_writestring("ATA: Secondary Slave detected\n");
        detected++;
    }
    
    drive_count = detected;
    return detected;
}

// Print information about detected drives
void ata_print_drive_info(void) {
    terminal_writestring("\n=== ATA Drive Information ===\n");
    
    for (int i = 0; i < drive_count; i++) {
        if (drives[i].exists) {
            terminal_writestring("Drive ");
            terminal_writestring(int_to_string(i));
            terminal_writestring(": ");
            
            if (drives[i].base_port == ATA_PRIMARY_BASE) {
                terminal_writestring("Primary ");
            } else {
                terminal_writestring("Secondary ");
            }
            
            if (drives[i].is_master) {
                terminal_writestring("Master\n");
            } else {
                terminal_writestring("Slave\n");
            }
            
            terminal_writestring("  Model: ");
            terminal_writestring(drives[i].model);
            terminal_writestring("\n");
            
            terminal_writestring("  Serial: ");
            terminal_writestring(drives[i].serial);
            terminal_writestring("\n");
            
            terminal_writestring("  Sectors: ");
            terminal_writestring(int_to_string(drives[i].sectors));
            terminal_writestring(" (");
            terminal_writestring(int_to_string(drives[i].sectors / 2048));
            terminal_writestring(" MB)\n\n");
        }
    }
}

// Phase 2: Safe disk reading implementation

// Get drive information by drive number
int ata_get_drive_info(uint8_t drive_num, ata_drive_t* info) {
    if (drive_num >= drive_count || !drives[drive_num].exists) {
        return 0;  // Drive doesn't exist
    }
    
    *info = drives[drive_num];
    return 1;  // Success
}

// Safely read sectors from ATA drive (read-only, no writing)
int ata_read_sectors(uint8_t drive_num, uint32_t lba, uint8_t sector_count, uint16_t* buffer) {
    // Safety checks
    if (drive_num >= drive_count || !drives[drive_num].exists) {
        terminal_writestring("ATA: Invalid drive number\n");
        return 0;
    }
    
    if (sector_count == 0 || sector_count > 128) {  // Reduced for safety
        terminal_writestring("ATA: Invalid sector count\n");
        return 0;
    }
    
    if (lba >= drives[drive_num].sectors) {
        terminal_writestring("ATA: LBA beyond drive capacity\n");
        return 0;
    }
    
    ata_drive_t* drive = &drives[drive_num];
    uint16_t base = drive->base_port;
    uint8_t drive_sel = drive->is_master ? ATA_DRIVE_MASTER : ATA_DRIVE_SLAVE;
    
    // Wait for drive to be ready
    ata_wait_ready(base);
    
    // Set up LBA addressing (LBA28 mode)
    outb(base + ATA_REG_FEATURES, 0x00);           // Features = 0
    outb(base + ATA_REG_SECCOUNT, sector_count);   // Sector count
    outb(base + ATA_REG_LBA_LOW, lba & 0xFF);      // LBA bits 0-7
    outb(base + ATA_REG_LBA_MID, (lba >> 8) & 0xFF);   // LBA bits 8-15
    outb(base + ATA_REG_LBA_HIGH, (lba >> 16) & 0xFF); // LBA bits 16-23
    
    // Drive selection with LBA bits 24-27
    uint8_t drive_head = drive_sel | 0x40 | ((lba >> 24) & 0x0F);
    outb(base + ATA_REG_DRIVE, drive_head);
    
    // Send READ SECTORS command
    outb(base + ATA_REG_COMMAND, ATA_CMD_READ_SECTORS);
    
    // Read sectors one by one
    for (int sector = 0; sector < sector_count; sector++) {
        // Wait for drive to be ready for data transfer
        int timeout = 0;
        uint8_t status;
        
        do {
            status = ata_read_status(base);
            if (status & ATA_STATUS_ERR) {
                terminal_writestring("ATA: Read error detected\n");
                return 0;
            }
            timeout++;
            if (timeout > 100000) {
                terminal_writestring("ATA: Read timeout\n");
                return 0;
            }
        } while (!(status & ATA_STATUS_DRQ) && (status & ATA_STATUS_BSY));
        
        // Read 256 words (512 bytes) for this sector
        uint16_t* sector_buffer = buffer + (sector * 256);
        for (int i = 0; i < 256; i++) {
            sector_buffer[i] = inw(base + ATA_REG_DATA);
        }
    }
    
    return 1;  // Success
}

// Test function to safely read first sector from first drive
int ata_test_read(void) {
    if (drive_count == 0) {
        terminal_writestring("ATA: No drives available for read test\n");
        return 0;
    }
    
    terminal_writestring("ATA: Testing safe sector read...\n");
    
    // Allocate buffer for one sector (512 bytes = 256 words)
    uint16_t* buffer = kmalloc(512);
    if (!buffer) {
        terminal_writestring("ATA: Failed to allocate buffer\n");
        return 0;
    }
    
    // Clear buffer
    for (int i = 0; i < 256; i++) {
        buffer[i] = 0;
    }
    
    // Try to read sector 0 from first drive
    int result = ata_read_sectors(0, 0, 1, buffer);
    
    if (result) {
        terminal_writestring("ATA: Read test successful!\n");
        terminal_writestring("ATA: First 32 bytes: ");
        
        // Display first 32 bytes in hex
        uint8_t* byte_buffer = (uint8_t*)buffer;
        for (int i = 0; i < 32; i++) {
            uint8_t byte_val = byte_buffer[i];
            uint8_t high_nibble = (byte_val >> 4) & 0x0F;
            uint8_t low_nibble = byte_val & 0x0F;
            
            char hex_char1 = (high_nibble < 10) ? ('0' + high_nibble) : ('A' + high_nibble - 10);
            char hex_char2 = (low_nibble < 10) ? ('0' + low_nibble) : ('A' + low_nibble - 10);
            
            terminal_putchar(hex_char1);
            terminal_putchar(hex_char2);
            terminal_putchar(' ');
        }
        terminal_writestring("\n");
    } else {
        terminal_writestring("ATA: Read test failed\n");
    }
    
    // Free buffer
    kfree(buffer);
    return result;
}

// Phase 3: Safe disk writing implementation

// Safely write sectors to ATA drive
int ata_write_sectors(uint8_t drive_num, uint32_t lba, uint8_t sector_count, const uint16_t* buffer) {
    // Safety checks
    if (drive_num >= drive_count || !drives[drive_num].exists) {
        terminal_writestring("ATA: Invalid drive number for write\n");
        return 0;
    }
    
    if (sector_count == 0 || sector_count > 64) {  // Conservative limit for writing
        terminal_writestring("ATA: Invalid sector count for write\n");
        return 0;
    }
    
    if (lba >= drives[drive_num].sectors) {
        terminal_writestring("ATA: LBA beyond drive capacity for write\n");
        return 0;
    }
    
    // Additional safety: Don't write to first few sectors (boot sectors)
    if (lba < 64) {
        terminal_writestring("ATA: Write to boot area prohibited for safety\n");
        return 0;
    }
    
    ata_drive_t* drive = &drives[drive_num];
    uint16_t base = drive->base_port;
    uint8_t drive_sel = drive->is_master ? ATA_DRIVE_MASTER : ATA_DRIVE_SLAVE;
    
    // Wait for drive to be ready
    ata_wait_ready(base);
    
    // Set up LBA addressing (LBA28 mode)
    outb(base + ATA_REG_FEATURES, 0x00);           // Features = 0
    outb(base + ATA_REG_SECCOUNT, sector_count);   // Sector count
    outb(base + ATA_REG_LBA_LOW, lba & 0xFF);      // LBA bits 0-7
    outb(base + ATA_REG_LBA_MID, (lba >> 8) & 0xFF);   // LBA bits 8-15
    outb(base + ATA_REG_LBA_HIGH, (lba >> 16) & 0xFF); // LBA bits 16-23
    
    // Drive selection with LBA bits 24-27
    uint8_t drive_head = drive_sel | 0x40 | ((lba >> 24) & 0x0F);
    outb(base + ATA_REG_DRIVE, drive_head);
    
    // Send WRITE SECTORS command
    outb(base + ATA_REG_COMMAND, ATA_CMD_WRITE_SECTORS);
    
    // Write sectors one by one
    for (int sector = 0; sector < sector_count; sector++) {
        // Wait for drive to be ready for data transfer
        int timeout = 0;
        uint8_t status;
        
        do {
            status = ata_read_status(base);
            if (status & ATA_STATUS_ERR) {
                terminal_writestring("ATA: Write error detected\n");
                return 0;
            }
            timeout++;
            if (timeout > 100000) {
                terminal_writestring("ATA: Write timeout\n");
                return 0;
            }
        } while (!(status & ATA_STATUS_DRQ) && (status & ATA_STATUS_BSY));
        
        // Write 256 words (512 bytes) for this sector
        const uint16_t* sector_buffer = buffer + (sector * 256);
        for (int i = 0; i < 256; i++) {
            outw(base + ATA_REG_DATA, sector_buffer[i]);
        }
        
        // Wait for write completion
        do {
            status = ata_read_status(base);
            if (status & ATA_STATUS_ERR) {
                terminal_writestring("ATA: Write completion error\n");
                return 0;
            }
        } while (status & ATA_STATUS_BSY);
    }
    
    return 1;  // Success
}

// Safe format function - creates a simple file system structure
int ata_format_drive(uint8_t drive_num) {
    if (drive_num >= drive_count || !drives[drive_num].exists) {
        terminal_writestring("ATA: Invalid drive number for format\n");
        return 0;
    }
    
    terminal_writestring("ATA: Formatting drive (creating ClaudeFS signature)...\n");
    
    // Allocate buffer for formatting
    uint16_t* format_buffer = kmalloc(512);
    if (!format_buffer) {
        terminal_writestring("ATA: Failed to allocate format buffer\n");
        return 0;
    }
    
    // Clear buffer
    for (int i = 0; i < 256; i++) {
        format_buffer[i] = 0;
    }
    
    // Create ClaudeFS signature at sector 64 (safe area)
    format_buffer[0] = 0x434C;  // "CL"
    format_buffer[1] = 0x4155;  // "AU"
    format_buffer[2] = 0x4445;  // "DE"
    format_buffer[3] = 0x4653;  // "FS"
    format_buffer[4] = 0x2020;  // "  "
    format_buffer[5] = 0x7631;  // "v1"
    
    // Write signature sector
    int result = ata_write_sectors(drive_num, 64, 1, format_buffer);
    
    kfree(format_buffer);
    
    if (result) {
        terminal_writestring("ATA: Drive formatting completed\n");
    } else {
        terminal_writestring("ATA: Drive formatting failed\n");
    }
    
    return result;
}

// Test function for write functionality
int ata_test_write(void) {
    if (drive_count == 0) {
        terminal_writestring("ATA: No drives available for write test\n");
        return 0;
    }
    
    terminal_writestring("ATA: Testing safe sector write...\n");
    
    // Allocate buffer for one sector (512 bytes = 256 words)
    uint16_t* write_buffer = kmalloc(512);
    uint16_t* read_buffer = kmalloc(512);
    
    if (!write_buffer || !read_buffer) {
        terminal_writestring("ATA: Failed to allocate test buffers\n");
        if (write_buffer) kfree(write_buffer);
        if (read_buffer) kfree(read_buffer);
        return 0;
    }
    
    // Create test pattern
    for (int i = 0; i < 256; i++) {
        write_buffer[i] = 0xAA55 + i;  // Test pattern
    }
    
    // Test LBA (safe area, beyond boot sectors)
    uint32_t test_lba = 100;
    
    // Write test data
    int write_result = ata_write_sectors(0, test_lba, 1, write_buffer);
    if (!write_result) {
        terminal_writestring("ATA: Write test failed\n");
        kfree(write_buffer);
        kfree(read_buffer);
        return 0;
    }
    
    // Read back and verify
    int read_result = ata_read_sectors(0, test_lba, 1, read_buffer);
    if (!read_result) {
        terminal_writestring("ATA: Read-back test failed\n");
        kfree(write_buffer);
        kfree(read_buffer);
        return 0;
    }
    
    // Compare data
    int verification_success = 1;
    for (int i = 0; i < 256; i++) {
        if (write_buffer[i] != read_buffer[i]) {
            verification_success = 0;
            break;
        }
    }
    
    if (verification_success) {
        terminal_writestring("ATA: Write/Read test successful - data integrity verified!\n");
    } else {
        terminal_writestring("ATA: Write/Read test failed - data corruption detected!\n");
    }
    
    // Clean up
    kfree(write_buffer);
    kfree(read_buffer);
    
    return verification_success;
}