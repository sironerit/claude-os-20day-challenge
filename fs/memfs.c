// ClaudeOS Memory-Based File System Implementation - Day 9
// Simple in-memory file system for testing and demonstration

#include "memfs.h"
#include "../kernel/kernel.h"
#include "../kernel/heap.h"

// Global file system state
static memfs_file_t file_table[MEMFS_MAX_FILES];
static memfs_fd_t fd_table[MEMFS_MAX_FD];
static bool memfs_initialized = false;
static uint32_t next_timestamp = 1;

// String utility functions for memfs
static size_t memfs_strlen(const char* str) {
    size_t len = 0;
    while (str[len] && len < MEMFS_MAX_FILENAME - 1) len++;
    return len;
}

static int memfs_strcmp(const char* str1, const char* str2) {
    while (*str1 && *str2 && *str1 == *str2) {
        str1++;
        str2++;
    }
    return *str1 - *str2;
}

static void memfs_strcpy(char* dest, const char* src, size_t max_len) {
    size_t i = 0;
    while (src[i] && i < max_len - 1) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

static void memfs_memcpy(void* dest, const void* src, size_t count) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    for (size_t i = 0; i < count; i++) {
        d[i] = s[i];
    }
}

static void memfs_memset(void* ptr, int value, size_t count) {
    uint8_t* p = (uint8_t*)ptr;
    for (size_t i = 0; i < count; i++) {
        p[i] = (uint8_t)value;
    }
}

// VGA helper function for memfs
static inline uint8_t vga_entry_color(vga_color fg, vga_color bg) {
    return fg | bg << 4;
}

// Initialize the memory file system
void memfs_init(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_MAGENTA, VGA_COLOR_BLACK));
    terminal_writestring("[MEMFS] Initializing memory-based file system...\n");
    
    // Clear file table
    for (int i = 0; i < MEMFS_MAX_FILES; i++) {
        memfs_memset(&file_table[i], 0, sizeof(memfs_file_t));
        file_table[i].in_use = false;
    }
    
    // Clear file descriptor table
    for (int i = 0; i < MEMFS_MAX_FD; i++) {
        memfs_memset(&fd_table[i], 0, sizeof(memfs_fd_t));
        fd_table[i].file_index = -1;
        fd_table[i].in_use = false;
    }
    
    // Create root directory (index 0)
    memfs_strcpy(file_table[0].name, "/", MEMFS_MAX_FILENAME);
    file_table[0].type = MEMFS_TYPE_DIR;
    file_table[0].size = 0;
    file_table[0].in_use = true;
    file_table[0].created_time = next_timestamp++;
    file_table[0].modified_time = file_table[0].created_time;
    
    // Create some demo files
    memfs_create("hello.txt");
    memfs_create("readme.md");
    memfs_create("test.txt");
    
    // Write demo content
    int fd = memfs_open("hello.txt", MEMFS_MODE_WRITE);
    if (fd >= 0) {
        memfs_write(fd, "Hello, ClaudeOS!\nThis is a test file in memory.\n", 49);
        memfs_close(fd);
    }
    
    fd = memfs_open("readme.md", MEMFS_MODE_WRITE);
    if (fd >= 0) {
        memfs_write(fd, "# ClaudeOS Memory File System\n\nThis is a simple in-memory file system.\n", 74);
        memfs_close(fd);
    }
    
    memfs_initialized = true;
    
    terminal_writestring("[MEMFS] Memory file system initialized successfully!\n");
    memfs_dump_stats();
}

// Find file by name
static int memfs_find_file(const char* filename) {
    if (!filename) return -1;
    
    for (int i = 0; i < MEMFS_MAX_FILES; i++) {
        if (file_table[i].in_use && 
            memfs_strcmp(file_table[i].name, filename) == 0) {
            return i;
        }
    }
    return -1;
}

// Find free file slot
static int memfs_find_free_file(void) {
    for (int i = 1; i < MEMFS_MAX_FILES; i++) { // Skip index 0 (root dir)
        if (!file_table[i].in_use) {
            return i;
        }
    }
    return -1;
}

// Find free file descriptor
static int memfs_find_free_fd(void) {
    for (int i = 0; i < MEMFS_MAX_FD; i++) {
        if (!fd_table[i].in_use) {
            return i;
        }
    }
    return -1;
}

// Validate filename
static bool memfs_valid_filename(const char* filename) {
    if (!filename || filename[0] == '\0') return false;
    if (memfs_strlen(filename) >= MEMFS_MAX_FILENAME) return false;
    
    // Check for invalid characters
    for (size_t i = 0; filename[i]; i++) {
        char c = filename[i];
        if (c < 32 || c > 126) return false; // Only printable ASCII
        if (c == '*' || c == '?' || c == '<' || c == '>' || c == '|') return false;
    }
    return true;
}

// Update file timestamps
static void memfs_update_timestamps(int file_index) {
    if (file_index >= 0 && file_index < MEMFS_MAX_FILES) {
        file_table[file_index].modified_time = next_timestamp++;
    }
}

// Create a new file
int memfs_create(const char* filename) {
    if (!memfs_initialized || !memfs_valid_filename(filename)) {
        return MEMFS_ERROR;
    }
    
    // Check if file already exists
    if (memfs_find_file(filename) >= 0) {
        return MEMFS_EXISTS;
    }
    
    // Find free slot
    int index = memfs_find_free_file();
    if (index < 0) {
        return MEMFS_NO_SPACE;
    }
    
    // Create file
    memfs_strcpy(file_table[index].name, filename, MEMFS_MAX_FILENAME);
    file_table[index].type = MEMFS_TYPE_FILE;
    file_table[index].size = 0;
    file_table[index].in_use = true;
    file_table[index].created_time = next_timestamp++;
    file_table[index].modified_time = file_table[index].created_time;
    
    memfs_memset(file_table[index].data, 0, MEMFS_MAX_FILESIZE);
    
    return MEMFS_SUCCESS;
}

// Open a file
int memfs_open(const char* filename, uint8_t mode) {
    if (!memfs_initialized || !filename) {
        return MEMFS_INVALID_FD;
    }
    
    // Find file
    int file_index = memfs_find_file(filename);
    if (file_index < 0) {
        return MEMFS_NOT_FOUND;
    }
    
    // Find free file descriptor
    int fd = memfs_find_free_fd();
    if (fd < 0) {
        return MEMFS_NO_SPACE;
    }
    
    // Setup file descriptor
    fd_table[fd].file_index = file_index;
    fd_table[fd].mode = mode;
    fd_table[fd].in_use = true;
    
    // Set position based on mode
    if (mode & MEMFS_MODE_APPEND) {
        fd_table[fd].position = file_table[file_index].size;
    } else {
        fd_table[fd].position = 0;
    }
    
    return fd;
}

// Close a file
int memfs_close(int fd) {
    if (fd < 0 || fd >= MEMFS_MAX_FD || !fd_table[fd].in_use) {
        return MEMFS_INVALID_FD;
    }
    
    // Clear file descriptor
    fd_table[fd].file_index = -1;
    fd_table[fd].position = 0;
    fd_table[fd].mode = 0;
    fd_table[fd].in_use = false;
    
    return MEMFS_SUCCESS;
}

// Read from file
int memfs_read(int fd, void* buffer, size_t count) {
    if (fd < 0 || fd >= MEMFS_MAX_FD || !fd_table[fd].in_use || !buffer) {
        return MEMFS_INVALID_FD;
    }
    
    if (!(fd_table[fd].mode & MEMFS_MODE_READ)) {
        return MEMFS_PERMISSION;
    }
    
    int file_index = fd_table[fd].file_index;
    if (file_index < 0 || !file_table[file_index].in_use) {
        return MEMFS_INVALID_FD;
    }
    
    // Calculate bytes to read
    size_t position = fd_table[fd].position;
    size_t file_size = file_table[file_index].size;
    size_t bytes_available = (position < file_size) ? (file_size - position) : 0;
    size_t bytes_to_read = (count < bytes_available) ? count : bytes_available;
    
    if (bytes_to_read == 0) {
        return 0; // EOF
    }
    
    // Copy data
    memfs_memcpy(buffer, &file_table[file_index].data[position], bytes_to_read);
    fd_table[fd].position += bytes_to_read;
    
    return bytes_to_read;
}

// Write to file
int memfs_write(int fd, const void* buffer, size_t count) {
    if (fd < 0 || fd >= MEMFS_MAX_FD || !fd_table[fd].in_use || !buffer) {
        return MEMFS_INVALID_FD;
    }
    
    if (!(fd_table[fd].mode & MEMFS_MODE_WRITE)) {
        return MEMFS_PERMISSION;
    }
    
    int file_index = fd_table[fd].file_index;
    if (file_index < 0 || !file_table[file_index].in_use) {
        return MEMFS_INVALID_FD;
    }
    
    size_t position = fd_table[fd].position;
    
    // Check if write would exceed file size limit
    if (position + count > MEMFS_MAX_FILESIZE) {
        count = MEMFS_MAX_FILESIZE - position;
        if (count == 0) {
            return MEMFS_NO_SPACE;
        }
    }
    
    // Copy data
    memfs_memcpy(&file_table[file_index].data[position], buffer, count);
    fd_table[fd].position += count;
    
    // Update file size if necessary
    if (fd_table[fd].position > file_table[file_index].size) {
        file_table[file_index].size = fd_table[fd].position;
    }
    
    memfs_update_timestamps(file_index);
    
    return count;
}

// List all files
int memfs_list(memfs_dirent_t* entries, size_t max_entries) {
    if (!entries || max_entries == 0) {
        return MEMFS_ERROR;
    }
    
    size_t count = 0;
    for (int i = 0; i < MEMFS_MAX_FILES && count < max_entries; i++) {
        if (file_table[i].in_use) {
            memfs_strcpy(entries[count].name, file_table[i].name, MEMFS_MAX_FILENAME);
            entries[count].type = file_table[i].type;
            entries[count].size = file_table[i].size;
            count++;
        }
    }
    
    return count;
}

// List files to terminal
void memfs_list_files(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("[MEMFS] File listing:\n");
    
    memfs_dirent_t entries[MEMFS_MAX_FILES];
    int count = memfs_list(entries, MEMFS_MAX_FILES);
    
    for (int i = 0; i < count; i++) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        if (entries[i].type == MEMFS_TYPE_DIR) {
            terminal_printf("  [DIR]  %s/\n", entries[i].name);
        } else {
            terminal_printf("  [FILE] %-20s (%d bytes)\n", entries[i].name, entries[i].size);
        }
    }
    
    if (count == 0) {
        terminal_writestring("  (no files)\n");
    }
}

// Get file system statistics
void memfs_get_stats(memfs_stats_t* stats) {
    if (!stats) return;
    
    stats->total_files = 0;
    stats->total_dirs = 0;
    stats->used_space = 0;
    
    for (int i = 0; i < MEMFS_MAX_FILES; i++) {
        if (file_table[i].in_use) {
            if (file_table[i].type == MEMFS_TYPE_FILE) {
                stats->total_files++;
                stats->used_space += file_table[i].size;
            } else {
                stats->total_dirs++;
            }
        }
    }
    
    stats->total_space = MEMFS_MAX_FILES * MEMFS_MAX_FILESIZE;
    stats->free_space = stats->total_space - stats->used_space;
}

// Display file system statistics
void memfs_dump_stats(void) {
    memfs_stats_t stats;
    memfs_get_stats(&stats);
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK));
    terminal_writestring("[MEMFS] File system statistics:\n");
    terminal_printf("  Files: %d, Directories: %d\n", stats.total_files, stats.total_dirs);
    terminal_printf("  Used space: %d bytes, Free space: %d bytes\n", 
                   stats.used_space, stats.free_space);
    terminal_printf("  Total capacity: %d bytes\n", stats.total_space);
}

// Check if file exists
int memfs_exists(const char* filename) {
    return (memfs_find_file(filename) >= 0) ? 1 : 0;
}

// Get file size
int memfs_size(const char* filename) {
    int index = memfs_find_file(filename);
    if (index >= 0) {
        return file_table[index].size;
    }
    return MEMFS_NOT_FOUND;
}

// Delete file
int memfs_delete(const char* filename) {
    if (!memfs_valid_filename(filename)) {
        return MEMFS_ERROR;
    }
    
    int index = memfs_find_file(filename);
    if (index < 0) {
        return MEMFS_NOT_FOUND;
    }
    
    if (index == 0) { // Cannot delete root directory
        return MEMFS_PERMISSION;
    }
    
    // Clear file entry
    memfs_memset(&file_table[index], 0, sizeof(memfs_file_t));
    file_table[index].in_use = false;
    
    return MEMFS_SUCCESS;
}