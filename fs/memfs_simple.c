// ClaudeOS Simple Memory File System - Day 9
// Basic in-memory file system without heap dependency

#include "memfs_simple.h"
#include "../kernel/kernel.h"

// Global file system state (Day 11 Enhanced)
static memfs_simple_file_t file_table[MEMFS_MAX_FILES];
static bool memfs_initialized = false;
static uint32_t next_file_id = 1;
static uint32_t current_dir_id = 0;  // Current directory (0 = root)
static uint32_t time_counter = 1000; // Simple timestamp counter

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

// Simple timestamp function (Day 11)
uint32_t memfs_simple_get_time(void) {
    return ++time_counter;
}

// Format timestamp for display (Phase 2)
void memfs_simple_format_time(uint32_t timestamp, char* buffer, size_t size) {
    if (!buffer || size < 8) return;
    
    // Simple format: "T1234"
    buffer[0] = 'T';
    uint32_t time = timestamp;
    int pos = 1;
    
    // Convert to string (max 4 digits for simplicity)
    if (time == 0) {
        buffer[pos++] = '0';
    } else {
        char temp[8];
        int temp_pos = 0;
        while (time > 0 && temp_pos < 4) {
            temp[temp_pos++] = '0' + (time % 10);
            time /= 10;
        }
        // Reverse
        for (int i = temp_pos - 1; i >= 0; i--) {
            if (pos < (int)size - 1) {
                buffer[pos++] = temp[i];
            }
        }
    }
    buffer[pos] = '\0';
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

// Initialize the memory file system (Day 11 Enhanced)
void memfs_simple_init(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_MAGENTA, VGA_COLOR_BLACK));
    terminal_writestring("[MEMFS] Initializing Day 11 enhanced memory file system...\n");
    
    // Clear file table
    for (int i = 0; i < MEMFS_MAX_FILES; i++) {
        simple_memset(&file_table[i], 0, sizeof(memfs_simple_file_t));
        file_table[i].in_use = false;
    }
    
    memfs_initialized = true;
    current_dir_id = 0;  // Start in root directory
    
    // Create root directory structure
    memfs_simple_mkdir("bin");
    memfs_simple_mkdir("home");
    memfs_simple_mkdir("tmp");
    
    // Create some demo files in root
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

// Find file by name with path support (Day 11 Enhanced)
int memfs_simple_find_file(const char* filename) {
    if (!filename) return -1;
    
    // Handle absolute path (starts with /)
    if (filename[0] == '/') {
        // Search in root directory
        for (int i = 0; i < MEMFS_MAX_FILES; i++) {
            if (file_table[i].in_use && 
                file_table[i].parent_id == 0 &&
                simple_strcmp(file_table[i].name, filename + 1) == 0) {
                return i;
            }
        }
        return -1;
    }
    
    // Search in current directory
    for (int i = 0; i < MEMFS_MAX_FILES; i++) {
        if (file_table[i].in_use && 
            file_table[i].parent_id == current_dir_id &&
            simple_strcmp(file_table[i].name, filename) == 0) {
            return i;
        }
    }
    return -1;
}

// Find file/directory in specific directory (Day 11)
int memfs_simple_find_in_dir(const char* name, uint32_t parent_id) {
    if (!name) return -1;
    
    for (int i = 0; i < MEMFS_MAX_FILES; i++) {
        if (file_table[i].in_use && 
            file_table[i].parent_id == parent_id &&
            simple_strcmp(file_table[i].name, name) == 0) {
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

// Create a new file (Day 11 Enhanced)
int memfs_simple_create(const char* filename) {
    if (!filename || simple_strlen(filename) == 0) {
        return MEMFS_ERROR;
    }
    
    // Check if file already exists in current directory
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
    file_table[index].type = MEMFS_TYPE_FILE;
    file_table[index].size = 0;
    file_table[index].in_use = true;
    file_table[index].id = next_file_id++;
    file_table[index].parent_id = current_dir_id;
    file_table[index].created_time = memfs_simple_get_time();
    file_table[index].modified_time = file_table[index].created_time;
    
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
    file_table[index].modified_time = memfs_simple_get_time();  // Update modification time
    
    return content_len;
}

// List all files in current directory (Day 11 Fixed)
void memfs_simple_list_files(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("[MEMFS] File listing for current directory:\n");
    
    int count = 0;
    for (int i = 0; i < MEMFS_MAX_FILES; i++) {
        if (file_table[i].in_use && file_table[i].parent_id == current_dir_id) {
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            
            // Type indicator and color
            if (file_table[i].type == MEMFS_TYPE_DIR) {
                terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK));
                terminal_writestring("  ");
                terminal_writestring(file_table[i].name);
                terminal_writestring("/ (directory)\n");
            } else {
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
            }
            count++;
        }
    }
    
    if (count == 0) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
        terminal_writestring("  (empty directory)\n");
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

// Directory operations (Day 11 New Functions)

// Create directory
int memfs_simple_mkdir(const char* dirname) {
    if (!dirname || simple_strlen(dirname) == 0) {
        return MEMFS_ERROR;
    }
    
    // Check if directory already exists
    if (memfs_simple_find_file(dirname) >= 0) {
        return MEMFS_EXISTS;
    }
    
    // Find free slot
    int index = memfs_simple_find_free_slot();
    if (index < 0) {
        return MEMFS_NO_SPACE;
    }
    
    // Create directory
    simple_strcpy(file_table[index].name, dirname, MEMFS_MAX_FILENAME);
    file_table[index].type = MEMFS_TYPE_DIR;
    file_table[index].size = 0;
    file_table[index].in_use = true;
    file_table[index].id = next_file_id++;
    file_table[index].parent_id = current_dir_id;
    file_table[index].created_time = memfs_simple_get_time();
    file_table[index].modified_time = file_table[index].created_time;
    
    simple_memset(file_table[index].data, 0, MEMFS_MAX_FILESIZE);
    
    return MEMFS_SUCCESS;
}

// Remove directory (only if empty)
int memfs_simple_rmdir(const char* dirname) {
    if (!dirname) {
        return MEMFS_ERROR;
    }
    
    int index = memfs_simple_find_file(dirname);
    if (index < 0) {
        return MEMFS_NOT_FOUND;
    }
    
    if (file_table[index].type != MEMFS_TYPE_DIR) {
        return MEMFS_NOT_DIR;
    }
    
    // Check if directory is empty
    uint32_t dir_id = file_table[index].id;
    for (int i = 0; i < MEMFS_MAX_FILES; i++) {
        if (file_table[i].in_use && file_table[i].parent_id == dir_id) {
            return MEMFS_ERROR; // Directory not empty
        }
    }
    
    // Clear directory entry
    simple_memset(&file_table[index], 0, sizeof(memfs_simple_file_t));
    file_table[index].in_use = false;
    
    return MEMFS_SUCCESS;
}

// Change directory
int memfs_simple_chdir(const char* dirname) {
    if (!dirname) {
        return MEMFS_ERROR;
    }
    
    // Handle special cases
    if (simple_strcmp(dirname, ".") == 0) {
        return MEMFS_SUCCESS; // Stay in current directory
    }
    
    if (simple_strcmp(dirname, "..") == 0) {
        // Go to parent directory
        if (current_dir_id != 0) {
            // Find current directory entry to get parent
            for (int i = 0; i < MEMFS_MAX_FILES; i++) {
                if (file_table[i].in_use && file_table[i].id == current_dir_id) {
                    current_dir_id = file_table[i].parent_id;
                    return MEMFS_SUCCESS;
                }
            }
        }
        return MEMFS_SUCCESS; // Already in root
    }
    
    if (simple_strcmp(dirname, "/") == 0) {
        current_dir_id = 0; // Go to root
        return MEMFS_SUCCESS;
    }
    
    // Find directory
    int index = memfs_simple_find_file(dirname);
    if (index < 0) {
        return MEMFS_NOT_FOUND;
    }
    
    if (file_table[index].type != MEMFS_TYPE_DIR) {
        return MEMFS_NOT_DIR;
    }
    
    current_dir_id = file_table[index].id;
    return MEMFS_SUCCESS;
}

// Get current working directory (Day 11 Enhanced)
void memfs_simple_getcwd(char* buffer, size_t size) {
    if (!buffer || size == 0) return;
    
    if (current_dir_id == 0) {
        // Root directory
        simple_strcpy(buffer, "/", size);
        return;
    }
    
    // Find current directory name and build path
    for (int i = 0; i < MEMFS_MAX_FILES; i++) {
        if (file_table[i].in_use && file_table[i].id == current_dir_id) {
            // Simple path for now: /dirname
            buffer[0] = '/';
            simple_strcpy(buffer + 1, file_table[i].name, size - 1);
            return;
        }
    }
    
    // Fallback
    simple_strcpy(buffer, "/", size);
}

// List files with detailed information (ls -l equivalent)
void memfs_simple_list_detailed(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("[MEMFS] Detailed file listing for current directory:\n");
    
    int count = 0;
    for (int i = 0; i < MEMFS_MAX_FILES; i++) {
        if (file_table[i].in_use && file_table[i].parent_id == current_dir_id) {
            // Type indicator
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            if (file_table[i].type == MEMFS_TYPE_DIR) {
                terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK));
                terminal_writestring("d ");
            } else {
                terminal_writestring("- ");
            }
            
            // Name
            terminal_writestring(file_table[i].name);
            terminal_writestring(" (");
            
            // Size
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
        terminal_writestring("  (empty directory)\n");
    }
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
}

// Advanced file operations (Phase 2)

// Touch file - create or update timestamp
int memfs_simple_touch(const char* filename) {
    if (!filename || simple_strlen(filename) == 0) {
        return MEMFS_ERROR;
    }
    
    int index = memfs_simple_find_file(filename);
    if (index >= 0) {
        // File exists, update timestamp
        file_table[index].modified_time = memfs_simple_get_time();
        return MEMFS_SUCCESS;
    } else {
        // File doesn't exist, create it
        return memfs_simple_create(filename);
    }
}

// Copy file
int memfs_simple_copy(const char* src, const char* dst) {
    if (!src || !dst) {
        return MEMFS_ERROR;
    }
    
    // Find source file
    int src_index = memfs_simple_find_file(src);
    if (src_index < 0) {
        return MEMFS_NOT_FOUND;
    }
    
    if (file_table[src_index].type != MEMFS_TYPE_FILE) {
        return MEMFS_ERROR; // Can't copy directories
    }
    
    // Check if destination exists
    if (memfs_simple_find_file(dst) >= 0) {
        return MEMFS_EXISTS;
    }
    
    // Create destination file
    int result = memfs_simple_create(dst);
    if (result != MEMFS_SUCCESS) {
        return result;
    }
    
    // Find destination file and copy data
    int dst_index = memfs_simple_find_file(dst);
    if (dst_index < 0) {
        return MEMFS_ERROR;
    }
    
    // Copy file data
    simple_memcpy(file_table[dst_index].data, file_table[src_index].data, file_table[src_index].size);
    file_table[dst_index].size = file_table[src_index].size;
    file_table[dst_index].modified_time = memfs_simple_get_time();
    
    return MEMFS_SUCCESS;
}

// Move/rename file
int memfs_simple_move(const char* src, const char* dst) {
    if (!src || !dst) {
        return MEMFS_ERROR;
    }
    
    // Find source file
    int src_index = memfs_simple_find_file(src);
    if (src_index < 0) {
        return MEMFS_NOT_FOUND;
    }
    
    // Check if destination exists
    if (memfs_simple_find_file(dst) >= 0) {
        return MEMFS_EXISTS;
    }
    
    // Simply rename by updating the name
    simple_strcpy(file_table[src_index].name, dst, MEMFS_MAX_FILENAME);
    file_table[src_index].modified_time = memfs_simple_get_time();
    
    return MEMFS_SUCCESS;
}

// Find file by name (search all directories) 
int memfs_simple_find(const char* name) {
    if (!name || simple_strlen(name) == 0) {
        return MEMFS_ERROR;
    }
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("[FIND] Searching for: ");
    terminal_writestring(name);
    terminal_writestring("\n");
    
    int found_count = 0;
    
    // Search all files in all directories
    for (int i = 0; i < MEMFS_MAX_FILES; i++) {
        if (file_table[i].in_use) {
            // Check if name matches (substring search)
            const char* filename = file_table[i].name;
            bool matches = false;
            
            // Simple substring search
            size_t name_len = simple_strlen(name);
            size_t file_len = simple_strlen(filename);
            
            if (name_len <= file_len) {
                for (size_t j = 0; j <= file_len - name_len; j++) {
                    bool match = true;
                    for (size_t k = 0; k < name_len; k++) {
                        if (filename[j + k] != name[k]) {
                            match = false;
                            break;
                        }
                    }
                    if (match) {
                        matches = true;
                        break;
                    }
                }
            }
            
            if (matches) {
                terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
                terminal_writestring("  Found: ");
                
                // Show directory path (simplified)
                if (file_table[i].parent_id == 0) {
                    terminal_writestring("/");
                } else {
                    // Find parent directory name
                    for (int j = 0; j < MEMFS_MAX_FILES; j++) {
                        if (file_table[j].in_use && file_table[j].id == file_table[i].parent_id) {
                            terminal_writestring("/");
                            terminal_writestring(file_table[j].name);
                            terminal_writestring("/");
                            break;
                        }
                    }
                }
                
                terminal_writestring(file_table[i].name);
                
                if (file_table[i].type == MEMFS_TYPE_DIR) {
                    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK));
                    terminal_writestring(" (directory)");
                } else {
                    terminal_writestring(" (file)");
                }
                terminal_writestring("\n");
                found_count++;
            }
        }
    }
    
    if (found_count == 0) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
        terminal_writestring("  No matches found\n");
    }
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    return found_count > 0 ? MEMFS_SUCCESS : MEMFS_NOT_FOUND;
}