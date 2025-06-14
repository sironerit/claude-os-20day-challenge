// ClaudeOS Virtual Memory Manager Implementation - Day 6
// Paging system for virtual memory management

#include "vmm.h"
#include "pmm.h"
#include "kernel.h"

// Current page directory
page_directory_t* current_page_directory = 0;

// Assembly function to load page directory (we'll implement this)
extern void vmm_load_page_directory(uint32_t page_dir_phys);
extern void vmm_enable_paging(void);
extern void vmm_flush_tlb(void);

// Get page table from page directory
static page_table_t* get_page_table(page_directory_t* dir, uint32_t virt_addr, int create) {
    uint32_t dir_index = GET_PAGE_DIR_INDEX(virt_addr);
    page_directory_entry_t* dir_entry = &dir->tables[dir_index];
    
    if (!dir_entry->present) {
        if (!create) {
            return 0;  // Page table doesn't exist
        }
        
        // Allocate new page table
        uint32_t table_phys = pmm_alloc_page();
        if (!table_phys) {
            return 0;  // Out of memory
        }
        
        // Clear the page table
        page_table_t* table = (page_table_t*)table_phys;
        for (int i = 0; i < PAGES_PER_TABLE; i++) {
            table->pages[i].present = 0;
            table->pages[i].writable = 0;
            table->pages[i].user = 0;
            table->pages[i].frame = 0;
        }
        
        // Set up directory entry
        dir_entry->present = 1;
        dir_entry->writable = 1;
        dir_entry->user = 1;
        dir_entry->table = table_phys >> 12;  // Physical frame number
        
        return table;
    }
    
    return (page_table_t*)(dir_entry->table << 12);
}

// Initialize virtual memory manager
void vmm_init(void) {
    terminal_writestring("VMM: Initializing virtual memory manager...\n");
    
    // Create kernel page directory
    uint32_t page_dir_phys = pmm_alloc_page();
    if (!page_dir_phys) {
        kernel_panic("VMM: Failed to allocate page directory");
    }
    
    current_page_directory = (page_directory_t*)page_dir_phys;
    
    // Clear page directory
    for (int i = 0; i < PAGES_PER_DIR; i++) {
        current_page_directory->tables[i].present = 0;
        current_page_directory->tables[i].writable = 0;
        current_page_directory->tables[i].user = 0;
        current_page_directory->tables[i].table = 0;
    }
    
    // Identity map first 4MB (kernel space)
    vmm_identity_map_kernel(current_page_directory);
    
    terminal_writestring("VMM: Virtual memory manager initialized\n");
}

// Create a new page directory
page_directory_t* vmm_create_page_directory(void) {
    uint32_t page_dir_phys = pmm_alloc_page();
    if (!page_dir_phys) {
        return 0;
    }
    
    page_directory_t* dir = (page_directory_t*)page_dir_phys;
    
    // Clear page directory
    for (int i = 0; i < PAGES_PER_DIR; i++) {
        dir->tables[i].present = 0;
        dir->tables[i].writable = 0;
        dir->tables[i].user = 0;
        dir->tables[i].table = 0;
    }
    
    return dir;
}

// Switch to different page directory
void vmm_switch_page_directory(page_directory_t* dir) {
    current_page_directory = dir;
    vmm_load_page_directory((uint32_t)dir);
}

// Map virtual address to physical address
void vmm_map_page(page_directory_t* dir, uint32_t virt_addr, uint32_t phys_addr, uint32_t flags) {
    page_table_t* table = get_page_table(dir, virt_addr, 1);
    if (!table) {
        return;  // Failed to get/create page table
    }
    
    uint32_t table_index = GET_PAGE_TABLE_INDEX(virt_addr);
    page_table_entry_t* page = &table->pages[table_index];
    
    page->present = (flags & PAGE_PRESENT) ? 1 : 0;
    page->writable = (flags & PAGE_WRITABLE) ? 1 : 0;
    page->user = (flags & PAGE_USER) ? 1 : 0;
    page->frame = phys_addr >> 12;  // Physical frame number
}

// Unmap virtual address
void vmm_unmap_page(page_directory_t* dir, uint32_t virt_addr) {
    page_table_t* table = get_page_table(dir, virt_addr, 0);
    if (!table) {
        return;  // Page table doesn't exist
    }
    
    uint32_t table_index = GET_PAGE_TABLE_INDEX(virt_addr);
    page_table_entry_t* page = &table->pages[table_index];
    
    page->present = 0;
    page->frame = 0;
    
    // Flush TLB for this page
    vmm_flush_tlb();
}

// Get physical address from virtual address
uint32_t vmm_get_physical_address(page_directory_t* dir, uint32_t virt_addr) {
    page_table_t* table = get_page_table(dir, virt_addr, 0);
    if (!table) {
        return 0;  // Page table doesn't exist
    }
    
    uint32_t table_index = GET_PAGE_TABLE_INDEX(virt_addr);
    page_table_entry_t* page = &table->pages[table_index];
    
    if (!page->present) {
        return 0;  // Page not present
    }
    
    return (page->frame << 12) | GET_PAGE_OFFSET(virt_addr);
}

// Check if page is present
int vmm_is_page_present(page_directory_t* dir, uint32_t virt_addr) {
    page_table_t* table = get_page_table(dir, virt_addr, 0);
    if (!table) {
        return 0;  // Page table doesn't exist
    }
    
    uint32_t table_index = GET_PAGE_TABLE_INDEX(virt_addr);
    return table->pages[table_index].present;
}

// Identity map kernel space (first 4MB)
void vmm_identity_map_kernel(page_directory_t* dir) {
    // Map first 4MB (1024 pages) with identity mapping
    for (uint32_t i = 0; i < 1024; i++) {
        uint32_t phys_addr = i * PAGE_SIZE;
        uint32_t virt_addr = phys_addr;
        
        vmm_map_page(dir, virt_addr, phys_addr, PAGE_PRESENT | PAGE_WRITABLE);
    }
    
    terminal_writestring("VMM: Kernel identity mapping complete (0-4MB)\n");
}