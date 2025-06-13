#ifndef MEMORY_H
#define MEMORY_H

#include "../kernel/types.h"

// Memory Management Subsystem
// Physical memory detection and basic allocation

// Memory map entry types (from multiboot)
#define MEMORY_TYPE_AVAILABLE   1
#define MEMORY_TYPE_RESERVED    2
#define MEMORY_TYPE_ACPI_RECLAIM 3
#define MEMORY_TYPE_ACPI_NVS    4
#define MEMORY_TYPE_BAD         5

// Memory constants
#define PAGE_SIZE           4096
#define PAGE_SIZE_BITS      12
#define KERNEL_START        0x100000    // 1MB - where kernel is loaded
#define MEMORY_MAP_MAX      32          // Maximum memory map entries

// Memory map entry structure
typedef struct {
    uint32_t base_low;      // Lower 32 bits of base address
    uint32_t base_high;     // Upper 32 bits of base address
    uint32_t length_low;    // Lower 32 bits of length
    uint32_t length_high;   // Upper 32 bits of length
    uint32_t type;          // Memory type
} __attribute__((packed)) memory_map_entry_t;

// Memory information structure
typedef struct {
    uint32_t total_memory;      // Total available memory in KB
    uint32_t usable_memory;     // Usable memory in KB
    uint32_t reserved_memory;   // Reserved memory in KB
    uint32_t kernel_end;        // End of kernel in memory
    uint32_t free_start;        // Start of free memory area
    uint8_t entry_count;        // Number of memory map entries
    memory_map_entry_t entries[MEMORY_MAP_MAX];
} memory_info_t;

// Global memory information
extern memory_info_t memory_info;

// Function declarations
void memory_init(uint32_t multiboot_flags, uint32_t memory_map_addr, uint32_t memory_map_length);
void memory_detect_multiboot(uint32_t memory_map_addr, uint32_t memory_map_length);
void memory_detect_bios(void);
uint32_t memory_get_total(void);
uint32_t memory_get_usable(void);
uint32_t memory_get_free_start(void);
void memory_print_map(void);

// Basic physical memory allocator (placeholder for future implementation)
uint32_t pmm_alloc_page(void);
void pmm_free_page(uint32_t page_addr);
void pmm_init(uint32_t start_addr, uint32_t end_addr);

#endif // MEMORY_H