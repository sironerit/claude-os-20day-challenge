#include "memory.h"

// VGA text output functions (from kernel)
extern void vga_write_string(const char* str);
extern void vga_write_char(char c);

// Global memory information
memory_info_t memory_info = {0};

// External kernel symbols (defined in linker script)
extern uint32_t kernel_start;
extern uint32_t kernel_end;

/**
 * Initialize memory management system
 */
void memory_init(uint32_t multiboot_flags, uint32_t memory_map_addr, uint32_t memory_map_length) {
    // Initialize memory info structure
    memory_info.total_memory = 0;
    memory_info.usable_memory = 0;
    memory_info.reserved_memory = 0;
    memory_info.kernel_end = (uint32_t)&kernel_end;
    memory_info.free_start = ((uint32_t)&kernel_end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1); // Page align
    memory_info.entry_count = 0;
    
    // Check if multiboot provided memory map
    if (multiboot_flags & 0x40) {
        // Use multiboot memory map
        memory_detect_multiboot(memory_map_addr, memory_map_length);
        vga_write_string("[MEM] Using multiboot memory map\n");
    } else {
        // Fall back to BIOS detection
        memory_detect_bios();
        vga_write_string("[MEM] Using BIOS memory detection\n");
    }
    
    vga_write_string("[MEM] Memory detection complete\n");
}

/**
 * Detect memory using multiboot memory map
 */
void memory_detect_multiboot(uint32_t memory_map_addr, uint32_t memory_map_length) {
    memory_map_entry_t* entry = (memory_map_entry_t*)memory_map_addr;
    uint32_t entry_size = 24; // Size of multiboot memory map entry
    uint32_t entries = memory_map_length / entry_size;
    
    // Limit entries to our maximum
    if (entries > MEMORY_MAP_MAX) {
        entries = MEMORY_MAP_MAX;
    }
    
    memory_info.entry_count = entries;
    
    // Process each memory map entry
    for (uint32_t i = 0; i < entries; i++) {
        // Copy entry to our structure
        memory_info.entries[i] = entry[i];
        
        // Calculate memory amounts (assuming 32-bit addresses for simplicity)
        uint32_t length_kb = entry[i].length_low / 1024;
        
        memory_info.total_memory += length_kb;
        
        switch (entry[i].type) {
            case MEMORY_TYPE_AVAILABLE:
                memory_info.usable_memory += length_kb;
                break;
            case MEMORY_TYPE_RESERVED:
            case MEMORY_TYPE_ACPI_RECLAIM:
            case MEMORY_TYPE_ACPI_NVS:
            case MEMORY_TYPE_BAD:
            default:
                memory_info.reserved_memory += length_kb;
                break;
        }
    }
}

/**
 * Detect memory using BIOS methods (fallback)
 */
void memory_detect_bios(void) {
    // Simple fallback - assume 16MB of memory with standard layout
    memory_info.entry_count = 2;
    
    // Entry 0: Low memory (0-640KB)
    memory_info.entries[0].base_low = 0x0;
    memory_info.entries[0].base_high = 0x0;
    memory_info.entries[0].length_low = 640 * 1024;
    memory_info.entries[0].length_high = 0x0;
    memory_info.entries[0].type = MEMORY_TYPE_AVAILABLE;
    
    // Entry 1: High memory (1MB-16MB)
    memory_info.entries[1].base_low = 0x100000;
    memory_info.entries[1].base_high = 0x0;
    memory_info.entries[1].length_low = 15 * 1024 * 1024;
    memory_info.entries[1].length_high = 0x0;
    memory_info.entries[1].type = MEMORY_TYPE_AVAILABLE;
    
    // Calculate totals
    memory_info.total_memory = 16 * 1024; // 16MB
    memory_info.usable_memory = (640 + 15 * 1024); // 640KB + 15MB
    memory_info.reserved_memory = 384; // 640KB-1MB reserved
}

/**
 * Get total memory in KB
 */
uint32_t memory_get_total(void) {
    return memory_info.total_memory;
}

/**
 * Get usable memory in KB
 */
uint32_t memory_get_usable(void) {
    return memory_info.usable_memory;
}

/**
 * Get start of free memory area
 */
uint32_t memory_get_free_start(void) {
    return memory_info.free_start;
}

/**
 * Print memory map (simplified to avoid screen flooding)
 */
void memory_print_map(void) {
    vga_write_string("[MEM] Memory detected: ");
    vga_write_char('0' + (memory_info.entry_count % 10));
    vga_write_string(" regions, memory available\n");
}

// Basic Physical Memory Manager (placeholder implementation)

static uint32_t next_free_page = 0;
static uint32_t memory_end = 0;

/**
 * Initialize physical memory manager
 */
void pmm_init(uint32_t start_addr, uint32_t end_addr) {
    next_free_page = (start_addr + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1); // Page align
    memory_end = end_addr & ~(PAGE_SIZE - 1); // Page align down
    
    vga_write_string("[PMM] Physical memory manager initialized\n");
}

/**
 * Allocate a physical page (4KB)
 * Returns page address or 0 if out of memory
 */
uint32_t pmm_alloc_page(void) {
    if (next_free_page >= memory_end) {
        return 0; // Out of memory
    }
    
    uint32_t page = next_free_page;
    next_free_page += PAGE_SIZE;
    return page;
}

/**
 * Free a physical page (placeholder - will implement proper free list later)
 */
void pmm_free_page(uint32_t page_addr) {
    // TODO: Implement proper free list
    // For now, just ignore the free operation
    (void)page_addr; // Suppress unused parameter warning
}