// ClaudeOS Kernel Heap Manager - Day 6
// Simple linked-list based kernel heap allocator

#ifndef HEAP_H
#define HEAP_H

#include "types.h"

// Heap configuration
#define HEAP_START          0x400000    // 4MB - start of kernel heap
#define HEAP_INITIAL_SIZE   0x100000    // 1MB - initial heap size
#define HEAP_MAX_SIZE       0x800000    // 8MB - maximum heap size

// Block header structure for free list
typedef struct block_header {
    size_t size;                    // Size of this block (excluding header)
    int is_free;                    // 1 if free, 0 if allocated
    struct block_header* next;      // Next block in free list
    struct block_header* prev;      // Previous block in free list
} block_header_t;

// Heap manager functions
void heap_init(void);
void* kmalloc(size_t size);
void kfree(void* ptr);
void* krealloc(void* ptr, size_t new_size);
void* kcalloc(size_t count, size_t size);

// Heap statistics and debugging
size_t heap_get_total_size(void);
size_t heap_get_used_size(void);
size_t heap_get_free_size(void);
void heap_dump_stats(void);
void heap_dump_blocks(void);

// Internal heap management
int heap_expand(size_t min_size);
void heap_coalesce_free_blocks(void);

#endif // HEAP_H