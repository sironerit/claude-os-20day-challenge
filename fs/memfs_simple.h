// ClaudeOS Simple Memory File System Header - Day 9
// Basic in-memory file system without complex dependencies

#ifndef MEMFS_SIMPLE_H
#define MEMFS_SIMPLE_H

#include "../kernel/types.h"

// File system constants
#define MEMFS_MAX_FILES 16          // Maximum number of files
#define MEMFS_MAX_FILENAME 32       // Maximum filename length
#define MEMFS_MAX_FILESIZE 512      // Maximum file size (512 bytes)

// Error codes
#define MEMFS_SUCCESS       0
#define MEMFS_ERROR        -1
#define MEMFS_NOT_FOUND    -2
#define MEMFS_EXISTS       -3
#define MEMFS_NO_SPACE     -4

// File entry structure
typedef struct memfs_simple_file {
    char name[MEMFS_MAX_FILENAME];      // File name
    size_t size;                        // Current file size
    uint8_t data[MEMFS_MAX_FILESIZE];   // File data
    bool in_use;                        // Entry in use flag
    uint32_t id;                        // Unique file ID
} memfs_simple_file_t;

// File system statistics
typedef struct memfs_simple_stats {
    uint32_t total_files;               // Total number of files
    uint32_t used_space;                // Total used space in bytes
    uint32_t free_space;                // Available space
    uint32_t total_space;               // Total available space
} memfs_simple_stats_t;

// Function prototypes

// File system initialization
void memfs_simple_init(void);
void memfs_simple_dump_stats(void);

// File operations
int memfs_simple_create(const char* filename);
int memfs_simple_delete(const char* filename);
int memfs_simple_exists(const char* filename);
int memfs_simple_get_size(const char* filename);

// File I/O (simplified)
int memfs_simple_read(const char* filename, char* buffer, size_t buffer_size);
int memfs_simple_write(const char* filename, const char* content);

// Utility functions
void memfs_simple_list_files(void);
void memfs_simple_get_stats(memfs_simple_stats_t* stats);

// Internal functions
int memfs_simple_find_file(const char* filename);

#endif // MEMFS_SIMPLE_H