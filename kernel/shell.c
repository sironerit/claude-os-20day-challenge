// ClaudeOS Shell - Day 11 Phase 1
// Simple command shell implementation

#include "shell.h"
#include "kernel.h"
#include "string.h"
#include "timer.h"

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

// Execute shell command
void shell_execute_command(const char* command) {
    // Remove leading/trailing whitespace
    while (*command == ' ' || *command == '\t') {
        command++;
    }
    
    if (strlen(command) == 0) {
        return;
    }
    
    // Built-in commands (Phase 1 - safe commands only)
    if (strcmp(command, "help") == 0) {
        cmd_help();
    } else if (strcmp(command, "clear") == 0) {
        cmd_clear();
    } else if (strcmp(command, "version") == 0) {
        cmd_version();
    } else if (strcmp(command, "uptime") == 0) {
        cmd_uptime();
    } else {
        terminal_setcolor(VGA_COLOR_LIGHT_RED);
        terminal_writestring("Command not found: ");
        terminal_writestring(command);
        terminal_writestring("\nType 'help' for available commands.\n");
        terminal_setcolor(VGA_COLOR_WHITE);
    }
}

// Built-in command implementations
void cmd_help(void) {
    terminal_setcolor(VGA_COLOR_LIGHT_CYAN);
    terminal_writestring("ClaudeOS Shell - Available Commands:\n");
    terminal_setcolor(VGA_COLOR_WHITE);
    terminal_writestring("  help     - Show this help message\n");
    terminal_writestring("  clear    - Clear the screen\n");
    terminal_writestring("  version  - Show OS version information\n");
    terminal_writestring("  uptime   - Show system uptime\n");
    terminal_writestring("\nMore commands coming in future updates!\n");
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