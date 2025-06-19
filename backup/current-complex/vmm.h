// ClaudeOS Virtual Memory Manager - Day 6
// Simple paging system for virtual memory management

#ifndef VMM_H
#define VMM_H

#include "types.h"

// Page directory and table entry flags
#define PAGE_PRESENT    0x001
#define PAGE_WRITABLE   0x002
#define PAGE_USER       0x004
#define PAGE_ACCESSED   0x020
#define PAGE_DIRTY      0x040

// Virtual memory constants
#define PAGES_PER_TABLE 1024
#define PAGES_PER_DIR   1024
#define PAGE_TABLE_SIZE 4096

// Get page directory/table indices from virtual address
#define GET_PAGE_DIR_INDEX(addr)   (((addr) >> 22) & 0x3FF)
#define GET_PAGE_TABLE_INDEX(addr) (((addr) >> 12) & 0x3FF)
#define GET_PAGE_OFFSET(addr)      ((addr) & 0xFFF)

// Page directory and table entry structures
typedef struct {
    uint32_t present    : 1;   // Page present in memory
    uint32_t writable   : 1;   // Page is writable
    uint32_t user       : 1;   // Page is user accessible
    uint32_t writethrough : 1; // Write-through caching
    uint32_t cache_disabled : 1; // Cache disabled
    uint32_t accessed   : 1;   // Page has been accessed
    uint32_t reserved   : 1;   // Reserved bit
    uint32_t page_size  : 1;   // Page size (0 = 4KB)
    uint32_t global     : 1;   // Global page
    uint32_t available  : 3;   // Available for OS use
    uint32_t frame      : 20;  // Physical frame address
} __attribute__((packed)) page_table_entry_t;

typedef struct {
    uint32_t present    : 1;   // Page table present
    uint32_t writable   : 1;   // Page table is writable
    uint32_t user       : 1;   // Page table is user accessible
    uint32_t writethrough : 1; // Write-through caching
    uint32_t cache_disabled : 1; // Cache disabled
    uint32_t accessed   : 1;   // Page table has been accessed
    uint32_t reserved   : 1;   // Reserved bit
    uint32_t page_size  : 1;   // Page size (0 = 4KB)
    uint32_t global     : 1;   // Global page
    uint32_t available  : 3;   // Available for OS use
    uint32_t table      : 20;  // Physical address of page table
} __attribute__((packed)) page_directory_entry_t;

// Page table and directory structures
typedef struct {
    page_table_entry_t pages[PAGES_PER_TABLE];
} page_table_t;

typedef struct {
    page_directory_entry_t tables[PAGES_PER_DIR];
} page_directory_t;

// Virtual memory manager functions
void vmm_init(void);
page_directory_t* vmm_create_page_directory(void);
void vmm_switch_page_directory(page_directory_t* dir);
void vmm_map_page(page_directory_t* dir, uint32_t virt_addr, uint32_t phys_addr, uint32_t flags);
void vmm_unmap_page(page_directory_t* dir, uint32_t virt_addr);
uint32_t vmm_get_physical_address(page_directory_t* dir, uint32_t virt_addr);
int vmm_is_page_present(page_directory_t* dir, uint32_t virt_addr);

// Identity mapping function for kernel
void vmm_identity_map_kernel(page_directory_t* dir);

// Assembly functions for paging operations
extern void vmm_load_page_directory(uint32_t page_dir_phys);
extern void vmm_enable_paging(void);
extern void vmm_flush_tlb(void);

// Current page directory
extern page_directory_t* current_page_directory;

#endif // VMM_H