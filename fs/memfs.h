// ClaudeOS Memory-Based File System - Day 9
// Simple in-memory file system for testing and demonstration

#ifndef MEMFS_H
#define MEMFS_H

#include "../kernel/types.h"

// File system constants (no hardcoding)
#define MEMFS_MAX_FILES 32          // Maximum number of files
#define MEMFS_MAX_FILENAME 64       // Maximum filename length
#define MEMFS_MAX_FILESIZE 4096     // Maximum file size (4KB)
#define MEMFS_MAX_DIRS 16           // Maximum number of directories
#define MEMFS_MAX_FD 32             // Maximum file descriptors

// File types
#define MEMFS_TYPE_FILE 1
#define MEMFS_TYPE_DIR  2

// File operation modes
#define MEMFS_MODE_READ   1
#define MEMFS_MODE_WRITE  2
#define MEMFS_MODE_APPEND 4

// Error codes
#define MEMFS_SUCCESS       0
#define MEMFS_ERROR        -1
#define MEMFS_NOT_FOUND    -2
#define MEMFS_EXISTS       -3
#define MEMFS_NO_SPACE     -4
#define MEMFS_INVALID_FD   -5
#define MEMFS_PERMISSION   -6

// File entry structure
typedef struct memfs_file {
    char name[MEMFS_MAX_FILENAME];      // File name
    uint8_t type;                       // File type (file or directory)
    size_t size;                        // Current file size
    uint8_t data[MEMFS_MAX_FILESIZE];   // File data
    bool in_use;                        // Entry in use flag
    uint32_t created_time;              // Creation timestamp
    uint32_t modified_time;             // Last modification timestamp
} memfs_file_t;

// File descriptor structure
typedef struct memfs_fd {
    int file_index;                     // Index into file table
    size_t position;                    // Current read/write position
    uint8_t mode;                       // Open mode (read/write/append)
    bool in_use;                        // FD in use flag
} memfs_fd_t;

// Directory entry for listing
typedef struct memfs_dirent {
    char name[MEMFS_MAX_FILENAME];      // Entry name
    uint8_t type;                       // Entry type
    size_t size;                        // Entry size
} memfs_dirent_t;

// File system statistics
typedef struct memfs_stats {
    uint32_t total_files;               // Total number of files
    uint32_t total_dirs;                // Total number of directories
    uint32_t used_space;                // Total used space in bytes
    uint32_t free_space;                // Available space
    uint32_t total_space;               // Total available space
} memfs_stats_t;

// Function prototypes

// File system initialization
void memfs_init(void);
void memfs_dump_stats(void);

// File operations
int memfs_open(const char* filename, uint8_t mode);
int memfs_close(int fd);
int memfs_read(int fd, void* buffer, size_t count);
int memfs_write(int fd, const void* buffer, size_t count);
int memfs_seek(int fd, size_t position);
int memfs_tell(int fd);

// File management
int memfs_create(const char* filename);
int memfs_delete(const char* filename);
int memfs_exists(const char* filename);
int memfs_size(const char* filename);

// Directory operations
int memfs_mkdir(const char* dirname);
int memfs_rmdir(const char* dirname);
int memfs_list(memfs_dirent_t* entries, size_t max_entries);

// Utility functions
void memfs_get_stats(memfs_stats_t* stats);
void memfs_list_files(void);
int memfs_copy(const char* source, const char* dest);

#endif // MEMFS_H