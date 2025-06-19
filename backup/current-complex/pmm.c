// ClaudeOS Physical Memory Manager Implementation - Day 6
// Bitmap-based physical page frame allocator

#include "pmm.h"
#include "kernel.h"

// Memory bitmap - each bit represents one 4KB page
static uint8_t memory_bitmap[BITMAP_SIZE];
static uint32_t total_pages;
static uint32_t free_pages;
static uint32_t first_free_page;

// Bitmap manipulation functions
static inline void set_bit(uint32_t bit) {
    memory_bitmap[bit / 8] |= (1 << (bit % 8));
}

static inline void clear_bit(uint32_t bit) {
    memory_bitmap[bit / 8] &= ~(1 << (bit % 8));
}

static inline int test_bit(uint32_t bit) {
    return memory_bitmap[bit / 8] & (1 << (bit % 8));
}

// Find first free page in bitmap
static uint32_t find_free_page(void) {
    for (uint32_t i = first_free_page; i < total_pages; i++) {
        if (!test_bit(i)) {
            return i;
        }
    }
    
    // Search from beginning if not found after first_free_page
    for (uint32_t i = 0; i < first_free_page; i++) {
        if (!test_bit(i)) {
            return i;
        }
    }
    
    return 0xFFFFFFFF;  // No free pages
}

// Initialize physical memory manager
void pmm_init(void) {
    total_pages = MEMORY_END / PAGE_SIZE;
    free_pages = total_pages;
    first_free_page = 0;
    
    // Clear bitmap (all pages initially free)
    for (uint32_t i = 0; i < BITMAP_SIZE; i++) {
        memory_bitmap[i] = 0;
    }
    
    // Mark kernel pages as used (0-1MB + kernel size)
    uint32_t kernel_end_page = PAGE_ALIGN(KERNEL_START + 0x100000) / PAGE_SIZE;  // Assume 1MB kernel max
    for (uint32_t i = 0; i < kernel_end_page; i++) {
        set_bit(i);
        free_pages--;
    }
    
    // Set first free page after kernel
    first_free_page = kernel_end_page;
    
    terminal_writestring("PMM: Physical Memory Manager initialized\n");
    terminal_writestring("PMM: Total pages: ");
    // Simple number printing
    char buffer[16];
    uint32_t num = total_pages;
    int pos = 0;
    if (num == 0) {
        buffer[pos++] = '0';
    } else {
        while (num > 0) {
            buffer[pos++] = '0' + (num % 10);
            num /= 10;
        }
    }
    // Reverse string
    for (int i = 0; i < pos / 2; i++) {
        char temp = buffer[i];
        buffer[i] = buffer[pos - 1 - i];
        buffer[pos - 1 - i] = temp;
    }
    buffer[pos] = '\0';
    terminal_writestring(buffer);
    terminal_writestring("\n");
    
    terminal_writestring("PMM: Free pages: ");
    num = free_pages;
    pos = 0;
    if (num == 0) {
        buffer[pos++] = '0';
    } else {
        while (num > 0) {
            buffer[pos++] = '0' + (num % 10);
            num /= 10;
        }
    }
    // Reverse string
    for (int i = 0; i < pos / 2; i++) {
        char temp = buffer[i];
        buffer[i] = buffer[pos - 1 - i];
        buffer[pos - 1 - i] = temp;
    }
    buffer[pos] = '\0';
    terminal_writestring(buffer);
    terminal_writestring("\n");
}

// Allocate a physical page (returns physical address)
uint32_t pmm_alloc_page(void) {
    if (free_pages == 0) {
        return 0;  // No free pages
    }
    
    uint32_t page = find_free_page();
    if (page == 0xFFFFFFFF) {
        return 0;  // No free pages found
    }
    
    // Mark page as used
    set_bit(page);
    free_pages--;
    
    // Update first_free_page hint
    if (page == first_free_page) {
        first_free_page++;
    }
    
    return PFN_TO_ADDR(page);
}

// Free a physical page
void pmm_free_page(uint32_t page_addr) {
    uint32_t page = ADDR_TO_PFN(page_addr);
    
    if (page >= total_pages) {
        return;  // Invalid page
    }
    
    if (!test_bit(page)) {
        return;  // Page already free
    }
    
    // Mark page as free
    clear_bit(page);
    free_pages++;
    
    // Update first_free_page hint
    if (page < first_free_page) {
        first_free_page = page;
    }
}

// Get memory statistics
uint32_t pmm_get_total_pages(void) {
    return total_pages;
}

uint32_t pmm_get_free_pages(void) {
    return free_pages;
}

uint32_t pmm_get_used_pages(void) {
    return total_pages - free_pages;
}

// Debug function to dump memory statistics
void pmm_dump_stats(void) {
    terminal_writestring("PMM Statistics:\n");
    terminal_writestring("  Total pages: ");
    
    // Simple number printing function
    char buffer[16];
    uint32_t num = total_pages;
    int pos = 0;
    if (num == 0) {
        buffer[pos++] = '0';
    } else {
        while (num > 0) {
            buffer[pos++] = '0' + (num % 10);
            num /= 10;
        }
    }
    // Reverse string
    for (int i = 0; i < pos / 2; i++) {
        char temp = buffer[i];
        buffer[i] = buffer[pos - 1 - i];
        buffer[pos - 1 - i] = temp;
    }
    buffer[pos] = '\0';
    terminal_writestring(buffer);
    terminal_writestring("\n");
    
    terminal_writestring("  Free pages: ");
    num = free_pages;
    pos = 0;
    if (num == 0) {
        buffer[pos++] = '0';
    } else {
        while (num > 0) {
            buffer[pos++] = '0' + (num % 10);
            num /= 10;
        }
    }
    for (int i = 0; i < pos / 2; i++) {
        char temp = buffer[i];
        buffer[i] = buffer[pos - 1 - i];
        buffer[pos - 1 - i] = temp;
    }
    buffer[pos] = '\0';
    terminal_writestring(buffer);
    terminal_writestring("\n");
    
    terminal_writestring("  Used pages: ");
    num = total_pages - free_pages;
    pos = 0;
    if (num == 0) {
        buffer[pos++] = '0';
    } else {
        while (num > 0) {
            buffer[pos++] = '0' + (num % 10);
            num /= 10;
        }
    }
    for (int i = 0; i < pos / 2; i++) {
        char temp = buffer[i];
        buffer[i] = buffer[pos - 1 - i];
        buffer[pos - 1 - i] = temp;
    }
    buffer[pos] = '\0';
    terminal_writestring(buffer);
    terminal_writestring("\n");
}