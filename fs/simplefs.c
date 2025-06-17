// ClaudeOS Simple File System Implementation
// Day 9 - In-memory file system with basic operations

#include "simplefs.h"
#include "../kernel/kernel.h"
#include "../kernel/heap.h"
#include "../kernel/string.h"

// Global file system state
fs_state_t g_fs_state;

// End of file marker for FAT
#define FAT_END_OF_FILE     0xFFFFFFFF

// Initialize the file system
int fs_init(void) {
    terminal_writestring("Initializing SimpleFS...\n");
    
    // Clear the file system state
    memset(&g_fs_state, 0, sizeof(fs_state_t));
    
    // Allocate memory for the entire file system
    size_t total_fs_size = SIMPLEFS_MAX_BLOCKS * SIMPLEFS_BLOCK_SIZE;
    g_fs_state.blocks = kmalloc(total_fs_size);
    
    if (!g_fs_state.blocks) {
        terminal_writestring("ERROR: Failed to allocate memory for file system\n");
        return FS_ERROR_NO_SPACE;
    }
    
    terminal_printf("Allocated %d KB for file system (%d blocks)\n", 
                   total_fs_size / 1024, SIMPLEFS_MAX_BLOCKS);
    
    // Clear all blocks
    memset(g_fs_state.blocks, 0, total_fs_size);
    
    // Set up pointers to specific blocks
    g_fs_state.superblock = (superblock_t*)fs_get_block(SUPERBLOCK_NUM);
    g_fs_state.fat = (fat_entry_t*)fs_get_block(FAT_BLOCK_NUM);
    
    // Format the file system
    int result = fs_format();
    if (result != FS_SUCCESS) {
        terminal_writestring("ERROR: Failed to format file system\n");
        kfree(g_fs_state.blocks);
        return result;
    }
    
    // Initialize current directory to root
    strcpy(g_fs_state.current_dir, "/");
    
    // Mark as initialized
    g_fs_state.initialized = 1;
    
    terminal_writestring("SimpleFS initialized successfully!\n");
    return FS_SUCCESS;
}

// Format the file system (create initial structures)
int fs_format(void) {
    // Initialize superblock
    superblock_t* sb = g_fs_state.superblock;
    sb->magic = SIMPLEFS_MAGIC;
    sb->total_blocks = SIMPLEFS_MAX_BLOCKS;
    sb->free_blocks = SIMPLEFS_MAX_BLOCKS - DATA_START_BLOCK_NUM; // Subtract system blocks
    sb->root_dir_block = ROOT_DIR_BLOCK_NUM;
    sb->fat_block = FAT_BLOCK_NUM;
    sb->data_start_block = DATA_START_BLOCK_NUM;
    sb->max_files = SIMPLEFS_MAX_FILES;
    sb->block_size = SIMPLEFS_BLOCK_SIZE;
    
    // Initialize FAT - mark system blocks as allocated
    fat_entry_t* fat = g_fs_state.fat;
    
    // Mark superblock, FAT, and root directory as allocated
    fat[SUPERBLOCK_NUM].allocated = 1;
    fat[SUPERBLOCK_NUM].next_block = FAT_END_OF_FILE;
    
    fat[FAT_BLOCK_NUM].allocated = 1;
    fat[FAT_BLOCK_NUM].next_block = FAT_END_OF_FILE;
    
    fat[ROOT_DIR_BLOCK_NUM].allocated = 1;
    fat[ROOT_DIR_BLOCK_NUM].next_block = FAT_END_OF_FILE;
    
    // Mark all other blocks as free
    for (uint32_t i = DATA_START_BLOCK_NUM; i < SIMPLEFS_MAX_BLOCKS; i++) {
        fat[i].allocated = 0;
        fat[i].next_block = 0;
    }
    
    // Initialize root directory (empty)
    dir_entry_t* root_dir = (dir_entry_t*)fs_get_block(ROOT_DIR_BLOCK_NUM);
    memset(root_dir, 0, SIMPLEFS_BLOCK_SIZE);
    
    terminal_writestring("File system formatted successfully\n");
    return FS_SUCCESS;
}

// Get pointer to a specific block
void* fs_get_block(uint32_t block_num) {
    if (block_num >= SIMPLEFS_MAX_BLOCKS) {
        return NULL;
    }
    
    char* base = (char*)g_fs_state.blocks;
    return base + (block_num * SIMPLEFS_BLOCK_SIZE);
}

// Allocate a free block
uint32_t fs_alloc_block(void) {
    fat_entry_t* fat = g_fs_state.fat;
    
    // Find first free block starting from data area
    for (uint32_t i = DATA_START_BLOCK_NUM; i < SIMPLEFS_MAX_BLOCKS; i++) {
        if (!fat[i].allocated) {
            fat[i].allocated = 1;
            fat[i].next_block = FAT_END_OF_FILE;
            g_fs_state.superblock->free_blocks--;
            return i;
        }
    }
    
    return 0; // No free blocks
}

// Free a block
int fs_free_block(uint32_t block_num) {
    if (block_num < DATA_START_BLOCK_NUM || block_num >= SIMPLEFS_MAX_BLOCKS) {
        return FS_ERROR_INVALID_PATH;
    }
    
    fat_entry_t* fat = g_fs_state.fat;
    if (!fat[block_num].allocated) {
        return FS_ERROR_NOT_FOUND; // Already free
    }
    
    fat[block_num].allocated = 0;
    fat[block_num].next_block = 0;
    g_fs_state.superblock->free_blocks++;
    
    return FS_SUCCESS;
}

// Check if a block is allocated
int fs_is_block_allocated(uint32_t block_num) {
    if (block_num >= SIMPLEFS_MAX_BLOCKS) {
        return 0;
    }
    
    return g_fs_state.fat[block_num].allocated;
}

// Find a directory entry by name
int fs_find_dir_entry(uint32_t dir_block, const char* name, dir_entry_t* entry) {
    dir_entry_t* dir = (dir_entry_t*)fs_get_block(dir_block);
    if (!dir) {
        return FS_ERROR_INVALID_PATH;
    }
    
    int entries_per_block = SIMPLEFS_BLOCK_SIZE / sizeof(dir_entry_t);
    
    for (int i = 0; i < entries_per_block; i++) {
        if (dir[i].name[0] != '\0' && strcmp(dir[i].name, name) == 0) {
            if (entry) {
                memcpy(entry, &dir[i], sizeof(dir_entry_t));
            }
            return i; // Return index
        }
    }
    
    return FS_ERROR_NOT_FOUND;
}

// Add a directory entry
int fs_add_dir_entry(uint32_t dir_block, const char* name, uint32_t first_block,
                     uint32_t size, uint8_t type) {
    dir_entry_t* dir = (dir_entry_t*)fs_get_block(dir_block);
    if (!dir) {
        return FS_ERROR_INVALID_PATH;
    }
    
    // Check if entry already exists
    if (fs_find_dir_entry(dir_block, name, NULL) >= 0) {
        return FS_ERROR_EXISTS;
    }
    
    int entries_per_block = SIMPLEFS_BLOCK_SIZE / sizeof(dir_entry_t);
    
    // Find first empty slot
    for (int i = 0; i < entries_per_block; i++) {
        if (dir[i].name[0] == '\0') {
            strncpy(dir[i].name, name, SIMPLEFS_MAX_FILENAME - 1);
            dir[i].name[SIMPLEFS_MAX_FILENAME - 1] = '\0';
            dir[i].first_block = first_block;
            dir[i].size = size;
            dir[i].type = type;
            return FS_SUCCESS;
        }
    }
    
    return FS_ERROR_NO_SPACE; // Directory full
}

// Remove a directory entry
int fs_remove_dir_entry(uint32_t dir_block, const char* name) {
    dir_entry_t* dir = (dir_entry_t*)fs_get_block(dir_block);
    if (!dir) {
        return FS_ERROR_INVALID_PATH;
    }
    
    int index = fs_find_dir_entry(dir_block, name, NULL);
    if (index < 0) {
        return FS_ERROR_NOT_FOUND;
    }
    
    // Clear the entry
    memset(&dir[index], 0, sizeof(dir_entry_t));
    
    return FS_SUCCESS;
}

// Allocate a file descriptor
int fs_alloc_fd(void) {
    for (int i = 0; i < SIMPLEFS_MAX_FD; i++) {
        if (!g_fs_state.fd_table[i].in_use) {
            memset(&g_fs_state.fd_table[i], 0, sizeof(file_descriptor_t));
            g_fs_state.fd_table[i].fd = i;
            g_fs_state.fd_table[i].in_use = 1;
            return i;
        }
    }
    return FS_ERROR_NO_FD;
}

// Free a file descriptor
void fs_free_fd(int fd) {
    if (fd >= 0 && fd < SIMPLEFS_MAX_FD) {
        g_fs_state.fd_table[fd].in_use = 0;
    }
}

// Get file descriptor structure
file_descriptor_t* fs_get_fd(int fd) {
    if (fd < 0 || fd >= SIMPLEFS_MAX_FD || !g_fs_state.fd_table[fd].in_use) {
        return NULL;
    }
    return &g_fs_state.fd_table[fd];
}

// Create a file or directory
int fs_create(const char* path, uint8_t type) {
    if (!g_fs_state.initialized) {
        return FS_ERROR_PERMISSION;
    }
    
    // For simplicity, only support files in root directory for now
    if (path[0] != '/' || strchr(path + 1, '/') != NULL) {
        return FS_ERROR_INVALID_PATH;
    }
    
    const char* filename = path + 1; // Skip leading '/'
    
    // Check if file already exists
    if (fs_find_dir_entry(ROOT_DIR_BLOCK_NUM, filename, NULL) >= 0) {
        return FS_ERROR_EXISTS;
    }
    
    // Allocate a block for the file
    uint32_t block = fs_alloc_block();
    if (block == 0) {
        return FS_ERROR_NO_SPACE;
    }
    
    // Add entry to root directory
    int result = fs_add_dir_entry(ROOT_DIR_BLOCK_NUM, filename, block, 0, type);
    if (result != FS_SUCCESS) {
        fs_free_block(block);
        return result;
    }
    
    // Initialize the block
    void* block_data = fs_get_block(block);
    memset(block_data, 0, SIMPLEFS_BLOCK_SIZE);
    
    return FS_SUCCESS;
}

// Open a file
int fs_open(const char* path, uint8_t mode) {
    if (!g_fs_state.initialized) {
        return FS_ERROR_PERMISSION;
    }
    
    // For simplicity, only support files in root directory for now
    if (path[0] != '/' || strchr(path + 1, '/') != NULL) {
        return FS_ERROR_INVALID_PATH;
    }
    
    const char* filename = path + 1; // Skip leading '/'
    
    // Find the file
    dir_entry_t entry;
    if (fs_find_dir_entry(ROOT_DIR_BLOCK_NUM, filename, &entry) < 0) {
        // File doesn't exist - create it if O_CREATE flag is set
        if (mode & O_CREATE) {
            int result = fs_create(path, FS_TYPE_FILE);
            if (result != FS_SUCCESS) {
                return result;
            }
            // Find the newly created file
            if (fs_find_dir_entry(ROOT_DIR_BLOCK_NUM, filename, &entry) < 0) {
                return FS_ERROR_NOT_FOUND;
            }
        } else {
            return FS_ERROR_NOT_FOUND;
        }
    }
    
    // Check if it's a directory
    if (entry.type == FS_TYPE_DIRECTORY) {
        return FS_ERROR_IS_DIR;
    }
    
    // Allocate file descriptor
    int fd = fs_alloc_fd();
    if (fd < 0) {
        return FS_ERROR_NO_FD;
    }
    
    file_descriptor_t* fdp = fs_get_fd(fd);
    fdp->first_block = entry.first_block;
    fdp->current_block = entry.first_block;
    fdp->position = 0;
    fdp->file_size = entry.size;
    fdp->mode = mode;
    
    // If truncate mode, reset file size
    if (mode & O_TRUNCATE) {
        fdp->file_size = 0;
        // Update directory entry
        dir_entry_t* dir = (dir_entry_t*)fs_get_block(ROOT_DIR_BLOCK_NUM);
        int index = fs_find_dir_entry(ROOT_DIR_BLOCK_NUM, filename, NULL);
        if (index >= 0) {
            dir[index].size = 0;
        }
    }
    
    return fd;
}

// Read from a file
int fs_read(int fd, void* buffer, uint32_t size) {
    file_descriptor_t* fdp = fs_get_fd(fd);
    if (!fdp) {
        return FS_ERROR_INVALID_FD;
    }
    
    if (!(fdp->mode & O_READ)) {
        return FS_ERROR_PERMISSION;
    }
    
    uint32_t bytes_read = 0;
    char* buf = (char*)buffer;
    
    while (bytes_read < size && fdp->position < fdp->file_size) {
        // Get current block
        void* block_data = fs_get_block(fdp->current_block);
        if (!block_data) {
            break;
        }
        
        // Calculate position within block
        uint32_t block_offset = fdp->position % SIMPLEFS_BLOCK_SIZE;
        uint32_t bytes_in_block = SIMPLEFS_BLOCK_SIZE - block_offset;
        uint32_t bytes_remaining = fdp->file_size - fdp->position;
        uint32_t bytes_to_read = size - bytes_read;
        
        if (bytes_to_read > bytes_in_block) {
            bytes_to_read = bytes_in_block;
        }
        if (bytes_to_read > bytes_remaining) {
            bytes_to_read = bytes_remaining;
        }
        
        // Copy data
        memcpy(buf + bytes_read, (char*)block_data + block_offset, bytes_to_read);
        bytes_read += bytes_to_read;
        fdp->position += bytes_to_read;
        
        // Move to next block if we've reached the end of current block
        if (fdp->position % SIMPLEFS_BLOCK_SIZE == 0 && 
            fdp->position < fdp->file_size) {
            uint32_t next_block = g_fs_state.fat[fdp->current_block].next_block;
            if (next_block == FAT_END_OF_FILE) {
                break; // End of file
            }
            fdp->current_block = next_block;
        }
    }
    
    return bytes_read;
}

// Write to a file
int fs_write(int fd, const void* buffer, uint32_t size) {
    file_descriptor_t* fdp = fs_get_fd(fd);
    if (!fdp) {
        return FS_ERROR_INVALID_FD;
    }
    
    if (!(fdp->mode & O_WRITE)) {
        return FS_ERROR_PERMISSION;
    }
    
    uint32_t bytes_written = 0;
    const char* buf = (const char*)buffer;
    
    while (bytes_written < size) {
        // Get current block
        void* block_data = fs_get_block(fdp->current_block);
        if (!block_data) {
            break;
        }
        
        // Calculate position within block
        uint32_t block_offset = fdp->position % SIMPLEFS_BLOCK_SIZE;
        uint32_t bytes_in_block = SIMPLEFS_BLOCK_SIZE - block_offset;
        uint32_t bytes_to_write = size - bytes_written;
        
        if (bytes_to_write > bytes_in_block) {
            bytes_to_write = bytes_in_block;
        }
        
        // Copy data
        memcpy((char*)block_data + block_offset, buf + bytes_written, bytes_to_write);
        bytes_written += bytes_to_write;
        fdp->position += bytes_to_write;
        
        // Update file size if we've extended the file
        if (fdp->position > fdp->file_size) {
            fdp->file_size = fdp->position;
        }
        
        // Check if we need to allocate a new block
        if (bytes_written < size && (fdp->position % SIMPLEFS_BLOCK_SIZE == 0)) {
            uint32_t next_block = g_fs_state.fat[fdp->current_block].next_block;
            if (next_block == FAT_END_OF_FILE) {
                // Allocate new block
                next_block = fs_alloc_block();
                if (next_block == 0) {
                    break; // No space
                }
                g_fs_state.fat[fdp->current_block].next_block = next_block;
            }
            fdp->current_block = next_block;
        }
    }
    
    return bytes_written;
}

// Close a file
int fs_close(int fd) {
    file_descriptor_t* fdp = fs_get_fd(fd);
    if (!fdp) {
        return FS_ERROR_INVALID_FD;
    }
    
    // Update directory entry with new file size if file was written to
    if (fdp->mode & O_WRITE) {
        // For simplicity, assume file is in root directory
        dir_entry_t* dir = (dir_entry_t*)fs_get_block(ROOT_DIR_BLOCK_NUM);
        int entries_per_block = SIMPLEFS_BLOCK_SIZE / sizeof(dir_entry_t);
        
        for (int i = 0; i < entries_per_block; i++) {
            if (dir[i].first_block == fdp->first_block && dir[i].type == FS_TYPE_FILE) {
                dir[i].size = fdp->file_size;
                break;
            }
        }
    }
    
    fs_free_fd(fd);
    return FS_SUCCESS;
}

// Simple mkdir implementation (creates in root directory only)
int fs_mkdir(const char* path) {
    return fs_create(path, FS_TYPE_DIRECTORY);
}

// List directory contents
int fs_list(const char* path, dir_entry_t* entries, int max_entries) {
    if (!g_fs_state.initialized) {
        return FS_ERROR_PERMISSION;
    }
    
    // For simplicity, only support root directory
    if (strcmp(path, "/") != 0) {
        return FS_ERROR_INVALID_PATH;
    }
    
    dir_entry_t* dir = (dir_entry_t*)fs_get_block(ROOT_DIR_BLOCK_NUM);
    int entries_per_block = SIMPLEFS_BLOCK_SIZE / sizeof(dir_entry_t);
    int count = 0;
    
    for (int i = 0; i < entries_per_block && count < max_entries; i++) {
        if (dir[i].name[0] != '\0') {
            if (entries) {
                memcpy(&entries[count], &dir[i], sizeof(dir_entry_t));
            }
            count++;
        }
    }
    
    return count;
}

// Dump file system statistics
void fs_dump_stats(void) {
    if (!g_fs_state.initialized) {
        terminal_writestring("File system not initialized\n");
        return;
    }
    
    superblock_t* sb = g_fs_state.superblock;
    
    terminal_writestring("SimpleFS Statistics:\n");
    terminal_printf("  Magic: 0x%x\n", sb->magic);
    terminal_printf("  Total blocks: %d\n", sb->total_blocks);
    terminal_printf("  Free blocks: %d\n", sb->free_blocks);
    terminal_printf("  Used blocks: %d\n", sb->total_blocks - sb->free_blocks);
    terminal_printf("  Block size: %d bytes\n", sb->block_size);
    terminal_printf("  Total size: %d KB\n", (sb->total_blocks * sb->block_size) / 1024);
    terminal_printf("  Free space: %d KB\n", (sb->free_blocks * sb->block_size) / 1024);
    
    // Count files and directories
    int file_count = 0;
    int dir_count = 0;
    
    dir_entry_t* root_dir = (dir_entry_t*)fs_get_block(ROOT_DIR_BLOCK_NUM);
    int entries_per_block = SIMPLEFS_BLOCK_SIZE / sizeof(dir_entry_t);
    
    for (int i = 0; i < entries_per_block; i++) {
        if (root_dir[i].name[0] != '\0') {
            if (root_dir[i].type == FS_TYPE_FILE) {
                file_count++;
            } else {
                dir_count++;
            }
        }
    }
    
    terminal_printf("  Files: %d\n", file_count);
    terminal_printf("  Directories: %d\n", dir_count);
    
    // File descriptor usage
    int fd_used = 0;
    for (int i = 0; i < SIMPLEFS_MAX_FD; i++) {
        if (g_fs_state.fd_table[i].in_use) {
            fd_used++;
        }
    }
    terminal_printf("  Open file descriptors: %d/%d\n", fd_used, SIMPLEFS_MAX_FD);
}

// Check if file system is initialized
int fs_is_initialized(void) {
    return g_fs_state.initialized;
}

// Cleanup file system
void fs_cleanup(void) {
    if (g_fs_state.blocks) {
        kfree(g_fs_state.blocks);
    }
    memset(&g_fs_state, 0, sizeof(fs_state_t));
}