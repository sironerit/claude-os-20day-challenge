// ClaudeOS Kernel Heap Manager Implementation - Day 6
// Simple linked-list based allocator for kernel memory

#include "heap.h"
#include "pmm.h"
#include "vmm.h"
#include "kernel.h"

// Heap state
static uint32_t heap_start = HEAP_START;
static uint32_t heap_end = 0;
static uint32_t heap_max = HEAP_START + HEAP_MAX_SIZE;
static block_header_t* free_list_head = 0;
int heap_initialized = 0;

// Simple memory functions
static void* memset(void* ptr, int value, size_t size) {
    uint8_t* p = (uint8_t*)ptr;
    for (size_t i = 0; i < size; i++) {
        p[i] = (uint8_t)value;
    }
    return ptr;
}

static void* memcpy(void* dest, const void* src, size_t size) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    for (size_t i = 0; i < size; i++) {
        d[i] = s[i];
    }
    return dest;
}

// Find free block that can fit the requested size
static block_header_t* find_free_block(size_t size) {
    block_header_t* current = free_list_head;
    
    while (current) {
        if (current->is_free && current->size >= size) {
            return current;
        }
        current = current->next;
    }
    
    return 0;  // No suitable block found
}

// Split a block if it's larger than needed
static void split_block(block_header_t* block, size_t size) {
    if (block->size <= size + sizeof(block_header_t) + 16) {
        return;  // Not worth splitting
    }
    
    // Create new block header after the allocated part
    block_header_t* new_block = (block_header_t*)((uint8_t*)block + sizeof(block_header_t) + size);
    new_block->size = block->size - size - sizeof(block_header_t);
    new_block->is_free = 1;
    new_block->next = block->next;
    new_block->prev = block;
    
    if (block->next) {
        block->next->prev = new_block;
    }
    block->next = new_block;
    
    // Update original block size
    block->size = size;
}

// Add block to free list
static void add_to_free_list(block_header_t* block) {
    block->is_free = 1;
    
    if (!free_list_head) {
        free_list_head = block;
        block->next = 0;
        block->prev = 0;
        return;
    }
    
    // Insert at beginning of free list
    block->next = free_list_head;
    block->prev = 0;
    free_list_head->prev = block;
    free_list_head = block;
}

// Remove block from free list
static void remove_from_free_list(block_header_t* block) {
    if (block->prev) {
        block->prev->next = block->next;
    } else {
        free_list_head = block->next;
    }
    
    if (block->next) {
        block->next->prev = block->prev;
    }
    
    block->is_free = 0;
    block->next = 0;
    block->prev = 0;
}

// Expand heap by allocating more pages
int heap_expand(size_t min_size) {
    if (heap_end + min_size > heap_max) {
        return 0;  // Would exceed maximum heap size
    }
    
    // Calculate how many pages we need
    size_t needed_size = (min_size + sizeof(block_header_t) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    size_t pages_needed = needed_size / PAGE_SIZE;
    
    // Allocate and map physical pages
    for (size_t i = 0; i < pages_needed; i++) {
        uint32_t phys_page = pmm_alloc_page();
        if (!phys_page) {
            return 0;  // Out of physical memory
        }
        
        uint32_t virt_addr = heap_end + (i * PAGE_SIZE);
        vmm_map_page(current_page_directory, virt_addr, phys_page, PAGE_PRESENT | PAGE_WRITABLE);
    }
    
    // Create new free block for the expanded area
    block_header_t* new_block = (block_header_t*)heap_end;
    new_block->size = needed_size - sizeof(block_header_t);
    new_block->is_free = 1;
    new_block->next = 0;
    new_block->prev = 0;
    
    heap_end += needed_size;
    
    // Add to free list
    add_to_free_list(new_block);
    
    return 1;  // Success
}

// Initialize heap
void heap_init(void) {
    // Check if VMM is initialized
    if (!current_page_directory) {
        terminal_writestring("HEAP: ERROR - VMM must be initialized first\n");
        return;
    }
    
    terminal_writestring("HEAP: Initializing kernel heap...\n");
    
    heap_end = heap_start + HEAP_INITIAL_SIZE;
    
    // Allocate initial heap pages
    size_t initial_pages = HEAP_INITIAL_SIZE / PAGE_SIZE;
    for (size_t i = 0; i < initial_pages; i++) {
        uint32_t phys_page = pmm_alloc_page();
        if (!phys_page) {
            kernel_panic("HEAP: Failed to allocate initial heap pages");
        }
        
        uint32_t virt_addr = heap_start + (i * PAGE_SIZE);
        vmm_map_page(current_page_directory, virt_addr, phys_page, PAGE_PRESENT | PAGE_WRITABLE);
    }
    
    // Create initial free block
    free_list_head = (block_header_t*)heap_start;
    free_list_head->size = HEAP_INITIAL_SIZE - sizeof(block_header_t);
    free_list_head->is_free = 1;
    free_list_head->next = 0;
    free_list_head->prev = 0;
    
    heap_initialized = 1;
    
    terminal_writestring("HEAP: Kernel heap initialized\n");
    terminal_writestring("HEAP: Start: 0x400000, Initial size: 1MB\n");
}

// Allocate memory
void* kmalloc(size_t size) {
    if (!heap_initialized) {
        return 0;
    }
    
    if (size == 0) {
        return 0;
    }
    
    // Align size to 8 bytes
    size = (size + 7) & ~7;
    
    // Find free block
    block_header_t* block = find_free_block(size);
    
    if (!block) {
        // Try to expand heap
        if (!heap_expand(size)) {
            return 0;  // Out of memory
        }
        block = find_free_block(size);
        if (!block) {
            return 0;
        }
    }
    
    // Remove from free list
    remove_from_free_list(block);
    
    // Split block if necessary
    split_block(block, size);
    
    // Return pointer to data (after header)
    return (void*)((uint8_t*)block + sizeof(block_header_t));
}

// Free memory
void kfree(void* ptr) {
    if (!ptr || !heap_initialized) {
        return;
    }
    
    // Get block header
    block_header_t* block = (block_header_t*)((uint8_t*)ptr - sizeof(block_header_t));
    
    // Validate block
    if ((uint32_t)block < heap_start || (uint32_t)block >= heap_end) {
        return;  // Invalid pointer
    }
    
    // Add to free list
    add_to_free_list(block);
    
    // Coalesce adjacent free blocks
    heap_coalesce_free_blocks();
}

// Reallocate memory
void* krealloc(void* ptr, size_t new_size) {
    if (!ptr) {
        return kmalloc(new_size);
    }
    
    if (new_size == 0) {
        kfree(ptr);
        return 0;
    }
    
    // Get current block
    block_header_t* block = (block_header_t*)((uint8_t*)ptr - sizeof(block_header_t));
    
    if (new_size <= block->size) {
        return ptr;  // Current block is large enough
    }
    
    // Allocate new block
    void* new_ptr = kmalloc(new_size);
    if (!new_ptr) {
        return 0;
    }
    
    // Copy data
    memcpy(new_ptr, ptr, block->size < new_size ? block->size : new_size);
    
    // Free old block
    kfree(ptr);
    
    return new_ptr;
}

// Allocate zeroed memory
void* kcalloc(size_t count, size_t size) {
    size_t total_size = count * size;
    void* ptr = kmalloc(total_size);
    if (ptr) {
        memset(ptr, 0, total_size);
    }
    return ptr;
}

// Coalesce adjacent free blocks
void heap_coalesce_free_blocks(void) {
    block_header_t* current = free_list_head;
    
    while (current && current->next) {
        block_header_t* next_block = current->next;
        
        // Check if blocks are adjacent
        uint8_t* current_end = (uint8_t*)current + sizeof(block_header_t) + current->size;
        if (current_end == (uint8_t*)next_block && next_block->is_free) {
            // Merge blocks
            current->size += sizeof(block_header_t) + next_block->size;
            current->next = next_block->next;
            if (next_block->next) {
                next_block->next->prev = current;
            }
        } else {
            current = current->next;
        }
    }
}

// Get heap statistics
size_t heap_get_total_size(void) {
    return heap_end - heap_start;
}

size_t heap_get_used_size(void) {
    size_t used = 0;
    uint8_t* current = (uint8_t*)heap_start;
    
    while (current < (uint8_t*)heap_end) {
        block_header_t* block = (block_header_t*)current;
        if (!block->is_free) {
            used += sizeof(block_header_t) + block->size;
        }
        current += sizeof(block_header_t) + block->size;
    }
    
    return used;
}

size_t heap_get_free_size(void) {
    return heap_get_total_size() - heap_get_used_size();
}

// Debug function to dump heap statistics
void heap_dump_stats(void) {
    terminal_writestring("HEAP Statistics:\n");
    terminal_writestring("  Total size: ");
    
    // Simple number printing
    char buffer[16];
    uint32_t num = heap_get_total_size();
    int pos = 0;
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
    terminal_writestring(" bytes\n");
    
    terminal_writestring("  Used size: ");
    num = heap_get_used_size();
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
    terminal_writestring(" bytes\n");
}