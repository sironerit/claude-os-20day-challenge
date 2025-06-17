#ifndef SIMPLEFS_H
#define SIMPLEFS_H

#include "../kernel/types.h"

// ClaudeOS Simple File System (SimpleFS)
// Day 9 Implementation - In-memory file system

// File System Constants
#define SIMPLEFS_MAGIC          0xC1ADEFU  // ClaudeFS magic number
#define SIMPLEFS_BLOCK_SIZE     4096        // 4KB blocks
#define SIMPLEFS_MAX_BLOCKS     1024        // Maximum blocks in FS
#define SIMPLEFS_MAX_FILES      256         // Maximum files per directory
#define SIMPLEFS_MAX_FILENAME   56          // Maximum filename length
#define SIMPLEFS_MAX_PATH       256         // Maximum path length
#define SIMPLEFS_MAX_FD         32          // Maximum open file descriptors

// File System Block Numbers
#define SUPERBLOCK_NUM          0           // Superblock at block 0
#define FAT_BLOCK_NUM           1           // FAT at block 1  
#define ROOT_DIR_BLOCK_NUM      2           // Root directory at block 2
#define DATA_START_BLOCK_NUM    3           // Data blocks start at block 3

// File Types
#define FS_TYPE_FILE            0           // Regular file
#define FS_TYPE_DIRECTORY       1           // Directory

// File Access Modes
#define O_READ                  0x01        // Read mode
#define O_WRITE                 0x02        // Write mode
#define O_CREATE                0x04        // Create if not exists
#define O_TRUNCATE              0x08        // Truncate to zero length

// Error Codes
#define FS_SUCCESS              0           // Operation successful
#define FS_ERROR_NOT_FOUND      -1          // File/directory not found
#define FS_ERROR_EXISTS         -2          // File/directory already exists
#define FS_ERROR_NO_SPACE       -3          // No space available
#define FS_ERROR_INVALID_PATH   -4          // Invalid path
#define FS_ERROR_NOT_DIR        -5          // Not a directory
#define FS_ERROR_IS_DIR         -6          // Is a directory
#define FS_ERROR_NO_FD          -7          // No file descriptors available
#define FS_ERROR_INVALID_FD     -8          // Invalid file descriptor
#define FS_ERROR_READ_ONLY      -9          // Read-only file system
#define FS_ERROR_PERMISSION     -10         // Permission denied

// Superblock Structure
typedef struct {
    uint32_t magic;             // Magic number (0xC1AUDEFS)
    uint32_t total_blocks;      // Total blocks in filesystem
    uint32_t free_blocks;       // Number of free blocks
    uint32_t root_dir_block;    // Block number of root directory
    uint32_t fat_block;         // Block number of FAT
    uint32_t data_start_block;  // First data block number
    uint32_t max_files;         // Maximum files per directory
    uint32_t block_size;        // Size of each block in bytes
    uint8_t  reserved[4072];    // Reserved space (pad to 4KB)
} __attribute__((packed)) superblock_t;

// File Allocation Table Entry
typedef struct {
    uint32_t next_block;        // Next block in file (0xFFFFFFFF = end)
    uint8_t  allocated;         // 1 = allocated, 0 = free
    uint8_t  reserved[3];       // Reserved for future use
} __attribute__((packed)) fat_entry_t;

// Directory Entry
typedef struct {
    char     name[SIMPLEFS_MAX_FILENAME];  // File/directory name
    uint32_t first_block;       // First block of file/directory data
    uint32_t size;              // File size in bytes (0 for directories)
    uint8_t  type;              // File type (FS_TYPE_FILE or FS_TYPE_DIRECTORY)
    uint8_t  reserved[3];       // Reserved for future use
} __attribute__((packed)) dir_entry_t;

// File Descriptor Structure
typedef struct {
    int32_t  fd;                // File descriptor number
    uint32_t first_block;       // First block of file
    uint32_t current_block;     // Current block for read/write
    uint32_t position;          // Current position in file (bytes)
    uint32_t file_size;         // Total file size
    uint8_t  mode;              // Access mode (O_READ, O_WRITE, etc.)
    uint8_t  in_use;            // 1 = active, 0 = available
    uint8_t  reserved[2];       // Reserved for future use
} file_descriptor_t;

// File System State
typedef struct {
    superblock_t* superblock;   // Pointer to superblock
    fat_entry_t*  fat;          // Pointer to FAT
    void*         blocks;       // Pointer to all blocks
    file_descriptor_t fd_table[SIMPLEFS_MAX_FD]; // File descriptor table
    char          current_dir[SIMPLEFS_MAX_PATH]; // Current directory path
    uint8_t       initialized;  // 1 = initialized, 0 = not initialized
} fs_state_t;

// Global file system state
extern fs_state_t g_fs_state;

// Core File System Functions

// Initialization and cleanup
int fs_init(void);
void fs_cleanup(void);
int fs_format(void);

// Block management
void* fs_get_block(uint32_t block_num);
uint32_t fs_alloc_block(void);
int fs_free_block(uint32_t block_num);
int fs_is_block_allocated(uint32_t block_num);

// Path and directory utilities
int fs_resolve_path(const char* path, char* resolved_path);
int fs_find_dir_entry(uint32_t dir_block, const char* name, dir_entry_t* entry);
int fs_add_dir_entry(uint32_t dir_block, const char* name, uint32_t first_block, 
                     uint32_t size, uint8_t type);
int fs_remove_dir_entry(uint32_t dir_block, const char* name);

// File descriptor management
int fs_alloc_fd(void);
void fs_free_fd(int fd);
file_descriptor_t* fs_get_fd(int fd);

// Core file operations
int fs_create(const char* path, uint8_t type);
int fs_open(const char* path, uint8_t mode);
int fs_read(int fd, void* buffer, uint32_t size);
int fs_write(int fd, const void* buffer, uint32_t size);
int fs_close(int fd);
int fs_delete(const char* path);

// Directory operations
int fs_mkdir(const char* path);
int fs_rmdir(const char* path);
int fs_list(const char* path, dir_entry_t* entries, int max_entries);
int fs_chdir(const char* path);
char* fs_getcwd(void);

// Utility functions
int fs_exists(const char* path);
int fs_is_directory(const char* path);
int fs_get_file_size(const char* path);
void fs_dump_stats(void);

// Debug functions
void fs_dump_superblock(void);
void fs_dump_fat(void);
void fs_dump_directory(uint32_t dir_block);

#endif // SIMPLEFS_H