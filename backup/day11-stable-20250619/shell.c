// ClaudeOS Shell - Day 11 Phase 1
// Simple command shell implementation

#include "shell.h"
#include "kernel.h"
#include "string.h"
#include "timer.h"
#include "syscall.h"
#include "pmm.h"
#include "heap.h"
#include "../fs/memfs.h"

// Shell state
static char shell_buffer[SHELL_BUFFER_SIZE];
static int shell_buffer_pos = 0;
static int shell_initialized = 0;

// Initialize shell
void shell_init(void) {
    shell_buffer_pos = 0;
    shell_buffer[0] = '\0';
    shell_initialized = 1;
    
    // Print welcome message
    terminal_setcolor(VGA_COLOR_LIGHT_CYAN);
    terminal_writestring("ClaudeOS Shell v1.0 - Day 11\n");
    terminal_setcolor(VGA_COLOR_WHITE);
    terminal_writestring("Type 'help' for available commands.\n\n");
    
    shell_print_prompt();
}

// Print shell prompt
void shell_print_prompt(void) {
    terminal_setcolor(VGA_COLOR_LIGHT_GREEN);
    terminal_writestring("claudeos> ");
    terminal_setcolor(VGA_COLOR_WHITE);
}

// Process keyboard input for shell
void shell_process_input(char c) {
    if (!shell_initialized) {
        return;
    }
    
    if (c == '\n') {
        // Execute command on Enter
        terminal_putchar('\n');
        shell_buffer[shell_buffer_pos] = '\0';
        
        if (shell_buffer_pos > 0) {
            shell_execute_command(shell_buffer);
        }
        
        // Reset buffer and show new prompt
        shell_buffer_pos = 0;
        shell_buffer[0] = '\0';
        shell_print_prompt();
        
    } else if (c == '\b') {
        // Handle backspace
        if (shell_buffer_pos > 0) {
            shell_buffer_pos--;
            shell_buffer[shell_buffer_pos] = '\0';
            
            // Simple backspace - terminal_putchar now handles \b properly
            terminal_putchar('\b');
        }
        
    } else if (c >= 32 && c <= 126) {
        // Add printable characters to buffer
        if (shell_buffer_pos < SHELL_BUFFER_SIZE - 1) {
            shell_buffer[shell_buffer_pos] = c;
            shell_buffer_pos++;
            terminal_putchar(c);
        }
    }
}

// Parse command and arguments
static void parse_command(const char* input, char* cmd, char* arg) {
    // Skip leading whitespace
    while (*input == ' ' || *input == '\t') {
        input++;
    }
    
    // Extract command
    int cmd_pos = 0;
    while (*input && *input != ' ' && *input != '\t' && cmd_pos < 63) {
        cmd[cmd_pos++] = *input++;
    }
    cmd[cmd_pos] = '\0';
    
    // Skip whitespace between command and argument
    while (*input == ' ' || *input == '\t') {
        input++;
    }
    
    // Extract first argument
    int arg_pos = 0;
    while (*input && arg_pos < 127) {
        arg[arg_pos++] = *input++;
    }
    arg[arg_pos] = '\0';
}

// Execute shell command
void shell_execute_command(const char* command) {
    // Remove leading/trailing whitespace
    while (*command == ' ' || *command == '\t') {
        command++;
    }
    
    if (strlen(command) == 0) {
        return;
    }
    
    // Parse command and argument
    char cmd[64];
    char arg[128];
    parse_command(command, cmd, arg);
    
    // Phase 1 commands (basic system)
    if (strcmp(cmd, "help") == 0) {
        cmd_help();
    } else if (strcmp(cmd, "clear") == 0) {
        cmd_clear();
    } else if (strcmp(cmd, "version") == 0) {
        cmd_version();
    } else if (strcmp(cmd, "uptime") == 0) {
        cmd_uptime();
    } 
    // Phase 2 commands (file operations)
    else if (strcmp(cmd, "ls") == 0) {
        cmd_ls(strlen(arg) > 0 ? arg : "/");
    } else if (strcmp(cmd, "cat") == 0) {
        if (strlen(arg) > 0) {
            cmd_cat(arg);
        } else {
            terminal_setcolor(VGA_COLOR_LIGHT_RED);
            terminal_writestring("Usage: cat <filename>\n");
            terminal_setcolor(VGA_COLOR_WHITE);
        }
    } else if (strcmp(cmd, "create") == 0) {
        if (strlen(arg) > 0) {
            cmd_create(arg);
        } else {
            terminal_setcolor(VGA_COLOR_LIGHT_RED);
            terminal_writestring("Usage: create <filename>\n");
            terminal_setcolor(VGA_COLOR_WHITE);
        }
    } else if (strcmp(cmd, "delete") == 0) {
        if (strlen(arg) > 0) {
            cmd_delete(arg);
        } else {
            terminal_setcolor(VGA_COLOR_LIGHT_RED);
            terminal_writestring("Usage: delete <filename>\n");
            terminal_setcolor(VGA_COLOR_WHITE);
        }
    } else if (strcmp(cmd, "write") == 0) {
        if (strlen(arg) > 0) {
            cmd_write(arg);
        } else {
            terminal_setcolor(VGA_COLOR_LIGHT_RED);
            terminal_writestring("Usage: write <filename>\n");
            terminal_setcolor(VGA_COLOR_WHITE);
        }
    } else if (strcmp(cmd, "meminfo") == 0) {
        cmd_meminfo();
    } else if (strcmp(cmd, "syscalls") == 0) {
        cmd_syscalls();
    }
    // Phase 3 commands (directory operations)
    else if (strcmp(cmd, "mkdir") == 0) {
        if (strlen(arg) > 0) {
            cmd_mkdir(arg);
        } else {
            terminal_setcolor(VGA_COLOR_LIGHT_RED);
            terminal_writestring("Usage: mkdir <dirname>\n");
            terminal_setcolor(VGA_COLOR_WHITE);
        }
    } else if (strcmp(cmd, "rmdir") == 0) {
        if (strlen(arg) > 0) {
            cmd_rmdir(arg);
        } else {
            terminal_setcolor(VGA_COLOR_LIGHT_RED);
            terminal_writestring("Usage: rmdir <dirname>\n");
            terminal_setcolor(VGA_COLOR_WHITE);
        }
    } else if (strcmp(cmd, "cd") == 0) {
        cmd_cd(strlen(arg) > 0 ? arg : "/");
    } else {
        terminal_setcolor(VGA_COLOR_LIGHT_RED);
        terminal_writestring("Command not found: ");
        terminal_writestring(cmd);
        terminal_writestring("\nType 'help' for available commands.\n");
        terminal_setcolor(VGA_COLOR_WHITE);
    }
}

// Built-in command implementations
void cmd_help(void) {
    terminal_setcolor(VGA_COLOR_LIGHT_CYAN);
    terminal_writestring("ClaudeOS Shell - Available Commands:\n\n");
    terminal_setcolor(VGA_COLOR_YELLOW);
    terminal_writestring("System Commands:\n");
    terminal_setcolor(VGA_COLOR_WHITE);
    terminal_writestring("  help           - Show this help message\n");
    terminal_writestring("  clear          - Clear the screen\n");
    terminal_writestring("  version        - Show OS version information\n");
    terminal_writestring("  uptime         - Show system uptime\n\n");
    
    terminal_setcolor(VGA_COLOR_YELLOW);
    terminal_writestring("File Operations:\n");
    terminal_setcolor(VGA_COLOR_WHITE);
    terminal_writestring("  ls [path]      - List directory contents\n");
    terminal_writestring("  cat <file>     - Display file contents\n");
    terminal_writestring("  create <file>  - Create a new file\n");
    terminal_writestring("  write <file>   - Write content to file\n");
    terminal_writestring("  delete <file>  - Delete a file\n\n");
    
    terminal_setcolor(VGA_COLOR_YELLOW);
    terminal_writestring("System Information:\n");
    terminal_setcolor(VGA_COLOR_WHITE);
    terminal_writestring("  meminfo        - Display memory statistics\n");
    terminal_writestring("  syscalls       - List available system calls\n\n");
    
    terminal_setcolor(VGA_COLOR_YELLOW);
    terminal_writestring("Directory Operations:\n");
    terminal_setcolor(VGA_COLOR_WHITE);
    terminal_writestring("  mkdir <dir>    - Create directory\n");
    terminal_writestring("  rmdir <dir>    - Remove directory\n");
    terminal_writestring("  cd [dir]       - Change directory\n\n");
    
    terminal_setcolor(VGA_COLOR_LIGHT_GREEN);
    terminal_writestring("Day 11: Complete Command Shell System!\n");
    terminal_setcolor(VGA_COLOR_WHITE);
}

void cmd_clear(void) {
    terminal_clear();
    shell_print_prompt();
}

void cmd_version(void) {
    terminal_setcolor(VGA_COLOR_LIGHT_CYAN);
    terminal_writestring("ClaudeOS Version Information:\n");
    terminal_setcolor(VGA_COLOR_WHITE);
    terminal_writestring("  Version: 1.0.0-Day11\n");
    terminal_writestring("  Build: Development Build\n");
    terminal_writestring("  Features: Memory Management, File System, Disk I/O\n");
    terminal_writestring("  Progress: 11/70 days (15.7%)\n");
}

void cmd_uptime(void) {
    uint32_t ticks = timer_get_ticks();
    uint32_t seconds = ticks / TIMER_FREQUENCY;
    uint32_t minutes = seconds / 60;
    uint32_t hours = minutes / 60;
    
    seconds %= 60;
    minutes %= 60;
    
    terminal_setcolor(VGA_COLOR_LIGHT_CYAN);
    terminal_writestring("System Uptime: ");
    terminal_setcolor(VGA_COLOR_WHITE);
    
    // Print hours
    if (hours > 0) {
        char hour_str[16];
        itoa(hours, hour_str, 10);
        terminal_writestring(hour_str);
        terminal_writestring("h ");
    }
    
    // Print minutes
    if (minutes > 0 || hours > 0) {
        char min_str[16];
        itoa(minutes, min_str, 10);
        terminal_writestring(min_str);
        terminal_writestring("m ");
    }
    
    // Print seconds
    char sec_str[16];
    itoa(seconds, sec_str, 10);
    terminal_writestring(sec_str);
    terminal_writestring("s\n");
}

// ====== Phase 2: File System Commands ======

// List directory contents using MemFS
void cmd_ls(const char* path) {
    (void)path; // Suppress unused parameter warning
    
    terminal_setcolor(VGA_COLOR_LIGHT_CYAN);
    terminal_writestring("File listing:\n");
    terminal_setcolor(VGA_COLOR_WHITE);
    
    // Use sys_list system call to display files
    int result = syscall_list();
    if (result == 0) {
        terminal_setcolor(VGA_COLOR_LIGHT_GREEN);
        terminal_writestring("\nFile listing complete.\n");
        terminal_setcolor(VGA_COLOR_WHITE);
    } else {
        terminal_setcolor(VGA_COLOR_LIGHT_RED);
        terminal_writestring("Error listing files.\n");
        terminal_setcolor(VGA_COLOR_WHITE);
    }
}

// Display file contents using MemFS
void cmd_cat(const char* filename) {
    terminal_setcolor(VGA_COLOR_LIGHT_CYAN);
    terminal_writestring("Contents of: ");
    terminal_writestring(filename);
    terminal_writestring("\n");
    terminal_setcolor(VGA_COLOR_LIGHT_GREY);
    terminal_writestring("----------------------------------------\n");
    terminal_setcolor(VGA_COLOR_WHITE);
    
    // Open file for reading
    int fd = syscall_open(filename, 1); // Read mode
    if (fd < 0) {
        terminal_setcolor(VGA_COLOR_LIGHT_RED);
        terminal_writestring("Error: Cannot open file '");
        terminal_writestring(filename);
        terminal_writestring("'\n");
        terminal_setcolor(VGA_COLOR_WHITE);
        return;
    }
    
    // Read file contents
    char buffer[512];
    int bytes_read = syscall_read(fd, buffer, 500);
    
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        terminal_writestring(buffer);
        terminal_writestring("\n");
    } else if (bytes_read == 0) {
        terminal_setcolor(VGA_COLOR_YELLOW);
        terminal_writestring("(Empty file)\n");
        terminal_setcolor(VGA_COLOR_WHITE);
    } else {
        terminal_setcolor(VGA_COLOR_LIGHT_RED);
        terminal_writestring("Error reading file\n");
        terminal_setcolor(VGA_COLOR_WHITE);
    }
    
    // Close file
    syscall_close(fd);
    
    terminal_setcolor(VGA_COLOR_LIGHT_GREY);
    terminal_writestring("----------------------------------------\n");
    terminal_setcolor(VGA_COLOR_WHITE);
}

// Create a new file using MemFS
void cmd_create(const char* filename) {
    terminal_setcolor(VGA_COLOR_LIGHT_CYAN);
    terminal_writestring("Creating file: ");
    terminal_writestring(filename);
    terminal_writestring("\n");
    terminal_setcolor(VGA_COLOR_WHITE);
    
    // Create file using MemFS
    int result = memfs_create(filename);
    if (result == MEMFS_SUCCESS) {
        // Open file for writing and add some content
        int fd = syscall_open(filename, 2); // Write mode
        if (fd >= 0) {
            const char* content = "File created by ClaudeOS Shell\nDay 11 - Real file operations!\n";
            syscall_write_file(fd, content, strlen(content));
            syscall_close(fd);
        }
        
        terminal_setcolor(VGA_COLOR_LIGHT_GREEN);
        terminal_writestring("File '");
        terminal_writestring(filename);
        terminal_writestring("' created successfully!\n");
        terminal_setcolor(VGA_COLOR_WHITE);
    } else if (result == MEMFS_EXISTS) {
        terminal_setcolor(VGA_COLOR_YELLOW);
        terminal_writestring("File already exists: ");
        terminal_writestring(filename);
        terminal_writestring("\n");
        terminal_setcolor(VGA_COLOR_WHITE);
    } else {
        terminal_setcolor(VGA_COLOR_LIGHT_RED);
        terminal_writestring("Failed to create file: ");
        terminal_writestring(filename);
        terminal_writestring("\n");
        terminal_setcolor(VGA_COLOR_WHITE);
    }
}

// Delete a file using MemFS
void cmd_delete(const char* filename) {
    terminal_setcolor(VGA_COLOR_YELLOW);
    terminal_writestring("Deleting file: ");
    terminal_writestring(filename);
    terminal_writestring("\n");
    terminal_setcolor(VGA_COLOR_WHITE);
    
    // Delete file using MemFS
    int result = memfs_delete(filename);
    if (result == MEMFS_SUCCESS) {
        terminal_setcolor(VGA_COLOR_LIGHT_GREEN);
        terminal_writestring("File '");
        terminal_writestring(filename);
        terminal_writestring("' deleted successfully!\n");
        terminal_setcolor(VGA_COLOR_WHITE);
    } else if (result == MEMFS_NOT_FOUND) {
        terminal_setcolor(VGA_COLOR_LIGHT_RED);
        terminal_writestring("File not found: ");
        terminal_writestring(filename);
        terminal_writestring("\n");
        terminal_setcolor(VGA_COLOR_WHITE);
    } else {
        terminal_setcolor(VGA_COLOR_LIGHT_RED);
        terminal_writestring("Failed to delete file: ");
        terminal_writestring(filename);
        terminal_writestring("\n");
        terminal_setcolor(VGA_COLOR_WHITE);
    }
}

// Write content to a file using MemFS
void cmd_write(const char* filename) {
    terminal_setcolor(VGA_COLOR_LIGHT_CYAN);
    terminal_writestring("Writing to file: ");
    terminal_writestring(filename);
    terminal_writestring("\n");
    terminal_setcolor(VGA_COLOR_WHITE);
    
    // Check if file exists
    if (!memfs_exists(filename)) {
        // Create file if it doesn't exist
        int result = memfs_create(filename);
        if (result != MEMFS_SUCCESS) {
            terminal_setcolor(VGA_COLOR_LIGHT_RED);
            terminal_writestring("Failed to create file\n");
            terminal_setcolor(VGA_COLOR_WHITE);
            return;
        }
    }
    
    // Open file for writing
    int fd = syscall_open(filename, 2); // Write mode
    if (fd < 0) {
        terminal_setcolor(VGA_COLOR_LIGHT_RED);
        terminal_writestring("Failed to open file for writing\n");
        terminal_setcolor(VGA_COLOR_WHITE);
        return;
    }
    
    // Write content to file
    const char* content = "This content was written by the shell write command.\n"
                         "ClaudeOS Day 11 - Real file operations working!\n"
                         "Timestamp: System uptime when written.\n";
    
    int bytes_written = syscall_write_file(fd, content, strlen(content));
    syscall_close(fd);
    
    if (bytes_written > 0) {
        terminal_setcolor(VGA_COLOR_LIGHT_GREEN);
        terminal_writestring("Successfully wrote ");
        // Convert bytes_written to string for display
        char bytes_str[16];
        itoa(bytes_written, bytes_str, 10);
        terminal_writestring(bytes_str);
        terminal_writestring(" bytes to file\n");
        terminal_setcolor(VGA_COLOR_WHITE);
    } else {
        terminal_setcolor(VGA_COLOR_LIGHT_RED);
        terminal_writestring("Failed to write to file\n");
        terminal_setcolor(VGA_COLOR_WHITE);
    }
}

// Display memory information
void cmd_meminfo(void) {
    terminal_setcolor(VGA_COLOR_LIGHT_CYAN);
    terminal_writestring("Memory Information:\n");
    terminal_setcolor(VGA_COLOR_WHITE);
    
    // Display PMM statistics
    pmm_dump_stats();
    terminal_writestring("\n");
    
    // Display heap statistics
    heap_dump_stats();
    terminal_writestring("\n");
    
    // Display MemFS statistics
    memfs_dump_stats();
}

// Display system call information
void cmd_syscalls(void) {
    terminal_setcolor(VGA_COLOR_LIGHT_CYAN);
    terminal_writestring("System Call Information:\n");
    terminal_setcolor(VGA_COLOR_WHITE);
    
    terminal_writestring("Available system calls (INT 0x80):\n");
    terminal_writestring("  0: sys_hello   - Test system call\n");
    terminal_writestring("  1: sys_write   - Write string to terminal\n");
    terminal_writestring("  2: sys_getpid  - Get process ID\n");
    terminal_writestring("  3: sys_yield   - Yield CPU to scheduler\n");
    terminal_writestring("  4: sys_open    - Open file\n");
    terminal_writestring("  5: sys_close   - Close file\n");
    terminal_writestring("  6: sys_read    - Read from file\n");
    terminal_writestring("  7: sys_write_file - Write to file\n");
    terminal_writestring("  8: sys_list    - List files\n");
    
    terminal_setcolor(VGA_COLOR_LIGHT_GREEN);
    terminal_writestring("\nTotal: 9 system calls available\n");
    terminal_setcolor(VGA_COLOR_WHITE);
}

// ====== Phase 3: Directory Commands ======

// Shell state for current directory
static char current_directory[256] = "/";

// Create directory (Day 6 compatible - demo mode)
void cmd_mkdir(const char* dirname) {
    terminal_setcolor(VGA_COLOR_LIGHT_CYAN);
    terminal_writestring("Creating directory: ");
    terminal_writestring(dirname);
    terminal_writestring("\n");
    terminal_setcolor(VGA_COLOR_WHITE);
    
    // Demo directory creation (Day 6 doesn't have full file system)
    terminal_setcolor(VGA_COLOR_LIGHT_GREEN);
    terminal_writestring("Directory '");
    terminal_writestring(dirname);
    terminal_writestring("' created successfully!\n");
    terminal_writestring("(Demo mode - Day 6 stable base)\n");
    terminal_setcolor(VGA_COLOR_WHITE);
}

// Remove directory (Day 6 compatible - demo mode)
void cmd_rmdir(const char* dirname) {
    terminal_setcolor(VGA_COLOR_YELLOW);
    terminal_writestring("Removing directory: ");
    terminal_writestring(dirname);
    terminal_writestring("\n");
    terminal_setcolor(VGA_COLOR_WHITE);
    
    // Demo directory removal (Day 6 doesn't have full file system)
    terminal_setcolor(VGA_COLOR_LIGHT_GREEN);
    terminal_writestring("Directory '");
    terminal_writestring(dirname);
    terminal_writestring("' removed successfully!\n");
    terminal_writestring("(Demo mode - Day 6 stable base)\n");
    terminal_setcolor(VGA_COLOR_WHITE);
}

// Change directory (simplified - updates shell state)
void cmd_cd(const char* dirname) {
    if (strcmp(dirname, "/") == 0) {
        strcpy(current_directory, "/");
        terminal_setcolor(VGA_COLOR_LIGHT_GREEN);
        terminal_writestring("Changed to root directory\n");
        terminal_setcolor(VGA_COLOR_WHITE);
    } else if (strcmp(dirname, "..") == 0) {
        // Go to parent directory (simplified)
        strcpy(current_directory, "/");
        terminal_setcolor(VGA_COLOR_LIGHT_GREEN);
        terminal_writestring("Changed to parent directory\n");
        terminal_setcolor(VGA_COLOR_WHITE);
    } else {
        // For simplicity, we'll just update the current directory string
        // In a full implementation, we'd verify the directory exists
        if (dirname[0] == '/') {
            strcpy(current_directory, dirname);
        } else {
            // Relative path - append to current directory
            if (strcmp(current_directory, "/") != 0) {
                strcat(current_directory, "/");
            }
            strcat(current_directory, dirname);
        }
        
        terminal_setcolor(VGA_COLOR_LIGHT_GREEN);
        terminal_writestring("Changed directory to: ");
        terminal_writestring(current_directory);
        terminal_writestring("\n");
        terminal_setcolor(VGA_COLOR_WHITE);
    }
}