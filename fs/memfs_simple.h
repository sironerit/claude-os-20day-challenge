// ClaudeOS Simple Memory File System Header - Day 9
// Basic in-memory file system without complex dependencies

#ifndef MEMFS_SIMPLE_H
#define MEMFS_SIMPLE_H

#include "../kernel/types.h"

// File system constants (Day 11 Phase 2 Enhanced)
#define MEMFS_MAX_FILES 32          // Maximum number of files/directories
#define MEMFS_MAX_FILENAME 32       // Maximum filename length
#define MEMFS_MAX_FILESIZE 16384    // Maximum file size (16KB for Day 18)
#define MEMFS_MAX_PATH 128          // Maximum path length

// File types (Day 11)
#define MEMFS_TYPE_FILE     1
#define MEMFS_TYPE_DIR      2

// File permissions (Day 18)
#define MEMFS_PERM_READ     0x004   // Read permission
#define MEMFS_PERM_WRITE    0x002   // Write permission
#define MEMFS_PERM_EXEC     0x001   // Execute permission
#define MEMFS_PERM_ALL      0x007   // All permissions (rwx)
#define MEMFS_PERM_DEFAULT  0x006   // Default permissions (rw-)

// Error codes
#define MEMFS_SUCCESS       0
#define MEMFS_ERROR        -1
#define MEMFS_NOT_FOUND    -2
#define MEMFS_EXISTS       -3
#define MEMFS_NO_SPACE     -4
#define MEMFS_NOT_DIR      -5
#define MEMFS_IS_DIR       -6

// File entry structure (Day 18 Enhanced)
typedef struct memfs_simple_file {
    char name[MEMFS_MAX_FILENAME];      // File/directory name
    uint8_t type;                       // File type (MEMFS_TYPE_FILE or MEMFS_TYPE_DIR)
    size_t size;                        // Current file size
    uint8_t data[MEMFS_MAX_FILESIZE];   // File data
    bool in_use;                        // Entry in use flag
    uint32_t id;                        // Unique file ID
    uint32_t parent_id;                 // Parent directory ID (0 = root)
    uint32_t created_time;              // Creation timestamp
    uint32_t modified_time;             // Last modification timestamp
    uint32_t accessed_time;             // Last access timestamp
    uint16_t permissions;               // File permissions (rwx format)
    uint16_t flags;                     // Additional file flags
    char owner[16];                     // File owner name
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

// Advanced file operations (Phase 2)
int memfs_simple_touch(const char* filename);
int memfs_simple_copy(const char* src, const char* dst);
int memfs_simple_move(const char* src, const char* dst);
int memfs_simple_find(const char* name);

// Directory operations (Day 11 New)
int memfs_simple_mkdir(const char* dirname);
int memfs_simple_rmdir(const char* dirname);
int memfs_simple_chdir(const char* dirname);
void memfs_simple_getcwd(char* buffer, size_t size);

// File I/O (simplified)
int memfs_simple_read(const char* filename, char* buffer, size_t buffer_size);
int memfs_simple_write(const char* filename, const char* content);

// Utility functions
void memfs_simple_list_files(void);
void memfs_simple_list_detailed(void);  // Day 11: ls -l equivalent
void memfs_simple_get_stats(memfs_simple_stats_t* stats);

// Day 18: File attribute management
int memfs_simple_chmod(const char* filename, uint16_t permissions);
int memfs_simple_chown(const char* filename, const char* owner);
int memfs_simple_stat(const char* filename, memfs_simple_file_t* stat_info);
void memfs_simple_format_permissions(uint16_t permissions, char* buffer);
void memfs_simple_show_file_info(const char* filename);

// Internal functions
int memfs_simple_find_file(const char* filename);
int memfs_simple_find_in_dir(const char* name, uint32_t parent_id);
uint32_t memfs_simple_get_time(void);  // Simple timestamp function
void memfs_simple_format_time(uint32_t timestamp, char* buffer, size_t size);  // Format timestamp for display

#endif // MEMFS_SIMPLE_H