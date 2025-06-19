// ClaudeOS Shell - Day 11 Phase 1
// Simple command shell implementation

#include "shell.h"
#include "kernel.h"
#include "string.h"
#include "timer.h"
#include "syscall.h"
#include "../fs/simplefs.h"

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
            
            // Visual backspace (move cursor back and clear character)
            terminal_putchar('\b');
            terminal_putchar(' ');
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
    terminal_writestring("  delete <file>  - Delete a file\n\n");
    
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

// List directory contents
void cmd_ls(const char* path) {
    terminal_setcolor(VGA_COLOR_LIGHT_CYAN);
    terminal_writestring("Directory listing for: ");
    terminal_writestring(path);
    terminal_writestring("\n");
    terminal_setcolor(VGA_COLOR_WHITE);
    
    // Use file system list syscall
    syscall_list(path);
}

// Display file contents
void cmd_cat(const char* filename) {
    terminal_setcolor(VGA_COLOR_LIGHT_CYAN);
    terminal_writestring("Contents of: ");
    terminal_writestring(filename);
    terminal_writestring("\n");
    terminal_setcolor(VGA_COLOR_LIGHT_GREY);
    terminal_writestring("----------------------------------------\n");
    terminal_setcolor(VGA_COLOR_WHITE);
    
    // Open file for reading
    int fd = syscall_open(filename, O_READ);
    if (fd < 0) {
        terminal_setcolor(VGA_COLOR_LIGHT_RED);
        terminal_writestring("Error: Could not open file '");
        terminal_writestring(filename);
        terminal_writestring("'\n");
        terminal_setcolor(VGA_COLOR_WHITE);
        return;
    }
    
    // Read and display file contents
    char buffer[1024];
    int bytes_read = syscall_read_file(fd, buffer, 1023);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        terminal_writestring(buffer);
        if (buffer[bytes_read-1] != '\n') {
            terminal_writestring("\n");
        }
    } else {
        terminal_setcolor(VGA_COLOR_YELLOW);
        terminal_writestring("(File is empty)\n");
        terminal_setcolor(VGA_COLOR_WHITE);
    }
    
    syscall_close(fd);
    terminal_setcolor(VGA_COLOR_LIGHT_GREY);
    terminal_writestring("----------------------------------------\n");
    terminal_setcolor(VGA_COLOR_WHITE);
}

// Create a new file
void cmd_create(const char* filename) {
    terminal_setcolor(VGA_COLOR_LIGHT_CYAN);
    terminal_writestring("Creating file: ");
    terminal_writestring(filename);
    terminal_writestring("\n");
    terminal_setcolor(VGA_COLOR_WHITE);
    
    // Create file with write mode
    int fd = syscall_open(filename, O_CREATE | O_WRITE);
    if (fd < 0) {
        terminal_setcolor(VGA_COLOR_LIGHT_RED);
        terminal_writestring("Error: Could not create file '");
        terminal_writestring(filename);
        terminal_writestring("'\n");
        terminal_setcolor(VGA_COLOR_WHITE);
        return;
    }
    
    // Write default content
    const char* default_content = "File created by ClaudeOS Shell\nEdit this file with your content.\n";
    int bytes_written = syscall_write_file(fd, default_content, strlen(default_content));
    syscall_close(fd);
    
    if (bytes_written > 0) {
        terminal_setcolor(VGA_COLOR_LIGHT_GREEN);
        terminal_writestring("File '");
        terminal_writestring(filename);
        terminal_writestring("' created successfully!\n");
        terminal_setcolor(VGA_COLOR_WHITE);
    } else {
        terminal_setcolor(VGA_COLOR_LIGHT_RED);
        terminal_writestring("Error: Could not write to file\n");
        terminal_setcolor(VGA_COLOR_WHITE);
    }
}

// Delete a file
void cmd_delete(const char* filename) {
    terminal_setcolor(VGA_COLOR_YELLOW);
    terminal_writestring("Deleting file: ");
    terminal_writestring(filename);
    terminal_writestring("\n");
    terminal_setcolor(VGA_COLOR_WHITE);
    
    // Use file system delete syscall
    int result = syscall_delete(filename);
    if (result == FS_SUCCESS) {
        terminal_setcolor(VGA_COLOR_LIGHT_GREEN);
        terminal_writestring("File '");
        terminal_writestring(filename);
        terminal_writestring("' deleted successfully!\n");
        terminal_setcolor(VGA_COLOR_WHITE);
    } else {
        terminal_setcolor(VGA_COLOR_LIGHT_RED);
        terminal_writestring("Error: Could not delete file '");
        terminal_writestring(filename);
        terminal_writestring("'\n");
        terminal_setcolor(VGA_COLOR_WHITE);
    }
}

// ====== Phase 3: Directory Commands ======

// Shell state for current directory
static char current_directory[256] = "/";

// Create directory
void cmd_mkdir(const char* dirname) {
    terminal_setcolor(VGA_COLOR_LIGHT_CYAN);
    terminal_writestring("Creating directory: ");
    terminal_writestring(dirname);
    terminal_writestring("\n");
    terminal_setcolor(VGA_COLOR_WHITE);
    
    // Use file system mkdir syscall
    int result = syscall_mkdir(dirname);
    if (result == FS_SUCCESS) {
        terminal_setcolor(VGA_COLOR_LIGHT_GREEN);
        terminal_writestring("Directory '");
        terminal_writestring(dirname);
        terminal_writestring("' created successfully!\n");
        terminal_setcolor(VGA_COLOR_WHITE);
    } else {
        terminal_setcolor(VGA_COLOR_LIGHT_RED);
        terminal_writestring("Error: Could not create directory '");
        terminal_writestring(dirname);
        terminal_writestring("'\n");
        terminal_setcolor(VGA_COLOR_WHITE);
    }
}

// Remove directory
void cmd_rmdir(const char* dirname) {
    terminal_setcolor(VGA_COLOR_YELLOW);
    terminal_writestring("Removing directory: ");
    terminal_writestring(dirname);
    terminal_writestring("\n");
    terminal_setcolor(VGA_COLOR_WHITE);
    
    // Use file system delete syscall (works for directories too)
    int result = syscall_delete(dirname);
    if (result == FS_SUCCESS) {
        terminal_setcolor(VGA_COLOR_LIGHT_GREEN);
        terminal_writestring("Directory '");
        terminal_writestring(dirname);
        terminal_writestring("' removed successfully!\n");
        terminal_setcolor(VGA_COLOR_WHITE);
    } else {
        terminal_setcolor(VGA_COLOR_LIGHT_RED);
        terminal_writestring("Error: Could not remove directory '");
        terminal_writestring(dirname);
        terminal_writestring("'\n");
        terminal_setcolor(VGA_COLOR_WHITE);
    }
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