// ClaudeOS Simple Memory File System - Day 9
// Basic in-memory file system without heap dependency

#include "memfs_simple.h"
#include "../kernel/kernel.h"

// Global file system state
static memfs_simple_file_t file_table[MEMFS_MAX_FILES];
static bool memfs_initialized = false;
static uint32_t next_file_id = 1;

// VGA helper function
static inline uint8_t vga_entry_color(vga_color fg, vga_color bg) {
    return fg | bg << 4;
}

// String utility functions
static size_t simple_strlen(const char* str) {
    size_t len = 0;
    while (str[len]) len++;  // Remove the MEMFS_MAX_FILENAME limit for content strings
    return len;
}

static int simple_strcmp(const char* str1, const char* str2) {
    while (*str1 && *str2 && *str1 == *str2) {
        str1++;
        str2++;
    }
    return *str1 - *str2;
}

static void simple_strcpy(char* dest, const char* src, size_t max_len) {
    size_t i = 0;
    while (src[i] && i < max_len - 1) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

static void simple_memcpy(void* dest, const void* src, size_t count) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    for (size_t i = 0; i < count; i++) {
        d[i] = s[i];
    }
}

static void simple_memset(void* ptr, int value, size_t count) {
    uint8_t* p = (uint8_t*)ptr;
    for (size_t i = 0; i < count; i++) {
        p[i] = (uint8_t)value;
    }
}

// Initialize the memory file system
void memfs_simple_init(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_MAGENTA, VGA_COLOR_BLACK));
    terminal_writestring("[MEMFS] Initializing simple memory file system...\n");
    
    // Clear file table
    for (int i = 0; i < MEMFS_MAX_FILES; i++) {
        simple_memset(&file_table[i], 0, sizeof(memfs_simple_file_t));
        file_table[i].in_use = false;
    }
    
    memfs_initialized = true;
    
    // Create some demo files
    memfs_simple_create("hello.txt");
    memfs_simple_create("readme.md");
    memfs_simple_create("test.txt");
    
    // Add demo content to hello.txt
    int index = memfs_simple_find_file("hello.txt");
    if (index >= 0) {
        const char* content = "Hello, ClaudeOS!\nThis is a test file in memory.\nMemFS Day 9 working!";
        size_t content_len = simple_strlen(content);
        
        // Clear data first and ensure proper copying
        simple_memset(file_table[index].data, 0, MEMFS_MAX_FILESIZE);
        
        if (content_len <= MEMFS_MAX_FILESIZE) {
            simple_memcpy(file_table[index].data, content, content_len);
            file_table[index].size = content_len;
            
            // Ensure null termination
            file_table[index].data[content_len] = '\0';
        }
    }
    terminal_writestring("[MEMFS] Simple memory file system initialized!\n");
    memfs_simple_list_files();
}

// Find file by name
int memfs_simple_find_file(const char* filename) {
    if (!filename) return -1;
    
    for (int i = 0; i < MEMFS_MAX_FILES; i++) {
        if (file_table[i].in_use && 
            simple_strcmp(file_table[i].name, filename) == 0) {
            return i;
        }
    }
    return -1;
}

// Find free file slot
static int memfs_simple_find_free_slot(void) {
    for (int i = 0; i < MEMFS_MAX_FILES; i++) {
        if (!file_table[i].in_use) {
            return i;
        }
    }
    return -1;
}

// Create a new file
int memfs_simple_create(const char* filename) {
    if (!filename || simple_strlen(filename) == 0) {
        return MEMFS_ERROR;
    }
    
    // Check if file already exists
    if (memfs_simple_find_file(filename) >= 0) {
        return MEMFS_EXISTS;
    }
    
    // Find free slot
    int index = memfs_simple_find_free_slot();
    if (index < 0) {
        return MEMFS_NO_SPACE;
    }
    
    // Create file
    simple_strcpy(file_table[index].name, filename, MEMFS_MAX_FILENAME);
    file_table[index].size = 0;
    file_table[index].in_use = true;
    file_table[index].id = next_file_id++;
    
    simple_memset(file_table[index].data, 0, MEMFS_MAX_FILESIZE);
    
    return MEMFS_SUCCESS;
}

// Delete a file
int memfs_simple_delete(const char* filename) {
    if (!filename) {
        return MEMFS_ERROR;
    }
    
    int index = memfs_simple_find_file(filename);
    if (index < 0) {
        return MEMFS_NOT_FOUND;
    }
    
    // Clear file entry
    simple_memset(&file_table[index], 0, sizeof(memfs_simple_file_t));
    file_table[index].in_use = false;
    
    return MEMFS_SUCCESS;
}

// Check if file exists
int memfs_simple_exists(const char* filename) {
    return (memfs_simple_find_file(filename) >= 0) ? 1 : 0;
}

// Get file size
int memfs_simple_get_size(const char* filename) {
    int index = memfs_simple_find_file(filename);
    if (index >= 0) {
        return file_table[index].size;
    }
    return -1;
}

// Read file content (simplified)
int memfs_simple_read(const char* filename, char* buffer, size_t buffer_size) {
    if (!filename || !buffer || buffer_size == 0) {
        return MEMFS_ERROR;
    }
    
    int index = memfs_simple_find_file(filename);
    if (index < 0) {
        return MEMFS_NOT_FOUND;
    }
    
    size_t file_size = file_table[index].size;
    size_t copy_size = (file_size < buffer_size - 1) ? file_size : buffer_size - 1;
    
    // Clear the buffer first
    simple_memset(buffer, 0, buffer_size);
    
    if (copy_size > 0) {
        simple_memcpy(buffer, file_table[index].data, copy_size);
    }
    
    // Ensure null termination
    buffer[copy_size] = '\0';
    
    return copy_size;
}

// Write file content (simplified)
int memfs_simple_write(const char* filename, const char* content) {
    if (!filename || !content) {
        return MEMFS_ERROR;
    }
    
    int index = memfs_simple_find_file(filename);
    if (index < 0) {
        // Create file if it doesn't exist
        if (memfs_simple_create(filename) != MEMFS_SUCCESS) {
            return MEMFS_ERROR;
        }
        index = memfs_simple_find_file(filename);
        if (index < 0) {
            return MEMFS_ERROR;
        }
    }
    
    size_t content_len = simple_strlen(content);
    if (content_len > MEMFS_MAX_FILESIZE) {
        content_len = MEMFS_MAX_FILESIZE;
    }
    
    simple_memcpy(file_table[index].data, content, content_len);
    file_table[index].size = content_len;
    
    return content_len;
}

// List all files
void memfs_simple_list_files(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("[MEMFS] File listing:\n");
    
    int count = 0;
    for (int i = 0; i < MEMFS_MAX_FILES; i++) {
        if (file_table[i].in_use) {
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            terminal_writestring("  ");
            terminal_writestring(file_table[i].name);
            terminal_writestring(" (");
            
            // Simple number printing
            char size_str[16];
            uint32_t size = file_table[i].size;
            int pos = 0;
            if (size == 0) {
                size_str[pos++] = '0';
            } else {
                while (size > 0) {
                    size_str[pos++] = '0' + (size % 10);
                    size /= 10;
                }
            }
            // Reverse string
            for (int j = 0; j < pos / 2; j++) {
                char temp = size_str[j];
                size_str[j] = size_str[pos - 1 - j];
                size_str[pos - 1 - j] = temp;
            }
            size_str[pos] = '\0';
            
            terminal_writestring(size_str);
            terminal_writestring(" bytes)\n");
            count++;
        }
    }
    
    if (count == 0) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
        terminal_writestring("  (no files)\n");
    }
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
}

// Get file system statistics
void memfs_simple_get_stats(memfs_simple_stats_t* stats) {
    if (!stats) return;
    
    stats->total_files = 0;
    stats->used_space = 0;
    
    for (int i = 0; i < MEMFS_MAX_FILES; i++) {
        if (file_table[i].in_use) {
            stats->total_files++;
            stats->used_space += file_table[i].size;
        }
    }
    
    stats->total_space = MEMFS_MAX_FILES * MEMFS_MAX_FILESIZE;
    stats->free_space = stats->total_space - stats->used_space;
}

// Display file system statistics
void memfs_simple_dump_stats(void) {
    memfs_simple_stats_t stats;
    memfs_simple_get_stats(&stats);
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK));
    terminal_writestring("[MEMFS] Simple MemFS statistics:\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    terminal_writestring("  Files: ");
    
    // Print total files
    char num_str[16];
    uint32_t num = stats.total_files;
    int pos = 0;
    if (num == 0) {
        num_str[pos++] = '0';
    } else {
        while (num > 0) {
            num_str[pos++] = '0' + (num % 10);
            num /= 10;
        }
    }
    for (int i = 0; i < pos / 2; i++) {
        char temp = num_str[i];
        num_str[i] = num_str[pos - 1 - i];
        num_str[pos - 1 - i] = temp;
    }
    num_str[pos] = '\0';
    terminal_writestring(num_str);
    
    terminal_writestring(", Used space: ");
    
    // Print used space
    num = stats.used_space;
    pos = 0;
    if (num == 0) {
        num_str[pos++] = '0';
    } else {
        while (num > 0) {
            num_str[pos++] = '0' + (num % 10);
            num /= 10;
        }
    }
    for (int i = 0; i < pos / 2; i++) {
        char temp = num_str[i];
        num_str[i] = num_str[pos - 1 - i];
        num_str[pos - 1 - i] = temp;
    }
    num_str[pos] = '\0';
    terminal_writestring(num_str);
    terminal_writestring(" bytes\n");
}