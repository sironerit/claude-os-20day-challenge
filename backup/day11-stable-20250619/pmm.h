// ClaudeOS Physical Memory Manager - Day 6
// Simple bitmap-based physical memory allocator

#ifndef PMM_H
#define PMM_H

#include "types.h"

// Memory constants
#define PAGE_SIZE 4096
#define PAGE_ALIGN(addr) (((addr) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#define PAGE_FLOOR(addr) ((addr) & ~(PAGE_SIZE - 1))

// Convert between addresses and page frame numbers
#define ADDR_TO_PFN(addr) ((addr) / PAGE_SIZE)
#define PFN_TO_ADDR(pfn) ((pfn) * PAGE_SIZE)

// Memory layout constants
#define KERNEL_START 0x100000     // 1MB - where kernel is loaded
#define MEMORY_END   0x2000000    // 32MB - maximum for our simple OS
#define BITMAP_SIZE  (MEMORY_END / PAGE_SIZE / 8)  // 1 bit per page

// Physical memory manager functions
void pmm_init(void);
uint32_t pmm_alloc_page(void);
void pmm_free_page(uint32_t page_addr);
uint32_t pmm_get_total_pages(void);
uint32_t pmm_get_free_pages(void);
uint32_t pmm_get_used_pages(void);

// Debug functions
void pmm_dump_stats(void);

#endif // PMM_H