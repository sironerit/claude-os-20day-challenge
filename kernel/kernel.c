// ClaudeOS Demo Kernel - Stable Day 6 Base + Basic Shell
// Simplified version for demonstration without complex memory management

#include "kernel.h"
#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "timer.h"
#include "keyboard.h"
#include "serial.h"
#include "pmm.h"
#include "syscall_simple.h"
#include "../fs/memfs_simple.h"

// VGA Text Mode Constants
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

// Variable argument list support
typedef __builtin_va_list va_list;
#define va_start(v,l) __builtin_va_start(v,l)
#define va_end(v) __builtin_va_end(v)
#define va_arg(v,l) __builtin_va_arg(v,l)

// Global variables
static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;

// VGA cursor management
void update_cursor(size_t x, size_t y) {
    uint16_t pos = y * VGA_WIDTH + x;
    
    // Tell the VGA board we are setting the high cursor byte
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    
    // Tell the VGA board we are setting the low cursor byte
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

// Advanced shell state
static char shell_buffer[256];
static int shell_pos = 0;

// Command parsing
#define MAX_ARGS 8
#define MAX_ARG_LEN 64
static char cmd_args[MAX_ARGS][MAX_ARG_LEN];
static int cmd_argc;

// VGA utility functions
static inline uint8_t vga_entry_color(vga_color fg, vga_color bg) {
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

// Terminal functions
void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_buffer = (uint16_t*) VGA_MEMORY;
    
    // Clear screen
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
    
    // Update hardware cursor
    update_cursor(terminal_column, terminal_row);
}

void terminal_setcolor(uint8_t color) {
    terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}

void terminal_scroll(void) {
    // Move all lines up by one
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            size_t src_index = (y + 1) * VGA_WIDTH + x;
            size_t dst_index = y * VGA_WIDTH + x;
            terminal_buffer[dst_index] = terminal_buffer[src_index];
        }
    }
    
    // Clear the last line
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        size_t index = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
        terminal_buffer[index] = vga_entry(' ', terminal_color);
    }
}

void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_scroll();
            terminal_row = VGA_HEIGHT - 1;
        }
        update_cursor(terminal_column, terminal_row);
        return;
    }
    
    if (c == '\b') {
        if (terminal_column > 0) {
            terminal_column--;
            terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
        }
        update_cursor(terminal_column, terminal_row);
        return;
    }
    
    terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
    if (++terminal_column == VGA_WIDTH) {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_scroll();
            terminal_row = VGA_HEIGHT - 1;
        }
    }
    update_cursor(terminal_column, terminal_row);
}

void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++)
        terminal_putchar(data[i]);
}

void terminal_writestring(const char* data) {
    size_t len = 0;
    while (data[len]) len++; // Simple strlen
    terminal_write(data, len);
}

void terminal_clear(void) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
    terminal_column = 0;
    terminal_row = 0;
    update_cursor(terminal_column, terminal_row);
}

// Simple shell functions
void shell_print_prompt(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("claudeos> ");
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
}

// Parse command line into arguments
void parse_command_line(const char* cmdline) {
    cmd_argc = 0;
    int i = 0;
    int arg_pos = 0;
    bool in_arg = false;
    
    // Clear previous arguments
    for (int j = 0; j < MAX_ARGS; j++) {
        cmd_args[j][0] = '\0';
    }
    
    while (cmdline[i] && cmd_argc < MAX_ARGS) {
        if (cmdline[i] == ' ' || cmdline[i] == '\t') {
            if (in_arg) {
                cmd_args[cmd_argc][arg_pos] = '\0';
                cmd_argc++;
                arg_pos = 0;
                in_arg = false;
            }
        } else {
            if (!in_arg) {
                in_arg = true;
            }
            if (arg_pos < MAX_ARG_LEN - 1) {
                cmd_args[cmd_argc][arg_pos] = cmdline[i];
                arg_pos++;
            }
        }
        i++;
    }
    
    // Add final argument if exists
    if (in_arg && cmd_argc < MAX_ARGS) {
        cmd_args[cmd_argc][arg_pos] = '\0';
        cmd_argc++;
    }
}

// String comparison helper
static int shell_strcmp(const char* str1, const char* str2) {
    while (*str1 && *str2 && *str1 == *str2) {
        str1++;
        str2++;
    }
    return *str1 - *str2;
}

void shell_process_command(const char* cmd) {
    // Parse command line into arguments
    parse_command_line(cmd);
    
    if (cmd_argc == 0) return;
    
    if (shell_strcmp(cmd_args[0], "help") == 0) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
        terminal_writestring("ClaudeOS Demo Shell - Available Commands:\n");
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        terminal_writestring("  help     - Show this help\n");
        terminal_writestring("  clear    - Clear screen\n");
        terminal_writestring("  version  - Show version\n");
        terminal_writestring("  hello    - Say hello\n");
        terminal_writestring("  demo     - Demo message\n");
        terminal_writestring("  meminfo  - Show memory statistics\n");
        terminal_writestring("  syscalls - Test system calls\n");
        terminal_writestring("  ls       - List files\n");
        terminal_writestring("  cat <file> - Display file content\n");
        terminal_writestring("  create <file> - Create new file\n");
        terminal_writestring("  delete <file> - Delete file\n");
        terminal_writestring("  write <file> <text> - Write to file\n");
        terminal_writestring("  fsinfo   - File system statistics\n\n");
    } else if (shell_strcmp(cmd_args[0], "clear") == 0) {
        terminal_clear();
        // Don't print prompt here - let the main loop handle it
        return;
    } else if (shell_strcmp(cmd_args[0], "version") == 0) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
        terminal_writestring("ClaudeOS Day 10 - Advanced Shell v1.0\n");
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        terminal_writestring("Enhanced with argument parsing and file operations\n");
    } else if (shell_strcmp(cmd_args[0], "hello") == 0) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
        terminal_writestring("Hello from ClaudeOS Shell!\n");
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    } else if (shell_strcmp(cmd_args[0], "demo") == 0) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_MAGENTA, VGA_COLOR_BLACK));
        terminal_writestring("Demo: Advanced shell with argument parsing!\n");
        terminal_writestring("Day 10 functionality working!\n");
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    } else if (shell_strcmp(cmd_args[0], "meminfo") == 0) {
        pmm_dump_stats();
    } else if (shell_strcmp(cmd_args[0], "syscalls") == 0) {
        test_syscalls();
    } else if (shell_strcmp(cmd_args[0], "ls") == 0) {
        memfs_simple_list_files();
    } else if (shell_strcmp(cmd_args[0], "cat") == 0) {
        if (cmd_argc < 2) {
            terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
            terminal_writestring("Usage: cat <filename>\n");
            terminal_writestring("Available files: hello.txt, readme.md, test.txt\n");
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        } else {
            terminal_setcolor(vga_entry_color(VGA_COLOR_CYAN, VGA_COLOR_BLACK));
            terminal_writestring("Displaying ");
            terminal_writestring(cmd_args[1]);
            
            // Debug: show file size
            int file_size = memfs_simple_get_size(cmd_args[1]);
            terminal_writestring(" (");
            char size_str[16];
            uint32_t size = (file_size > 0) ? (uint32_t)file_size : 0;
            int pos = 0;
            if (size == 0) {
                size_str[pos++] = '0';
            } else {
                while (size > 0) {
                    size_str[pos++] = '0' + (size % 10);
                    size /= 10;
                }
            }
            for (int i = 0; i < pos / 2; i++) {
                char temp = size_str[i];
                size_str[i] = size_str[pos - 1 - i];
                size_str[pos - 1 - i] = temp;
            }
            size_str[pos] = '\0';
            terminal_writestring(size_str);
            terminal_writestring(" bytes):\n");
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            
            char buffer[256];
            // Clear buffer first
            for (int i = 0; i < 256; i++) {
                buffer[i] = '\0';
            }
            
            int result = memfs_simple_read(cmd_args[1], buffer, 256);
            if (result > 0) {
                // Process and display the content
                for (int i = 0; i < result; i++) {
                    char c = buffer[i];
                    if (c == '\0') break;  // Stop at null terminator
                    if (c == '\n') {
                        terminal_putchar('\n');
                    } else if (c >= 32 && c <= 126) {  // Printable ASCII only
                        terminal_putchar(c);
                    }
                }
                terminal_putchar('\n');
            } else {
                terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
                terminal_writestring("File not found or read error\n");
                terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            }
        }
    } else if (shell_strcmp(cmd_args[0], "create") == 0) {
        if (cmd_argc < 2) {
            terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
            terminal_writestring("Usage: create <filename>\n");
            terminal_writestring("Example: create myfile.txt\n");
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        } else {
            terminal_setcolor(vga_entry_color(VGA_COLOR_CYAN, VGA_COLOR_BLACK));
            terminal_writestring("Creating file: ");
            terminal_writestring(cmd_args[1]);
            terminal_writestring("\n");
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            
            int result = memfs_simple_create(cmd_args[1]);
            if (result == MEMFS_SUCCESS) {
                memfs_simple_write(cmd_args[1], "This is a newly created file!\nDay 10 Advanced Shell working!");
                terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
                terminal_writestring("File created successfully!\n");
                terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            } else if (result == MEMFS_EXISTS) {
                terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
                terminal_writestring("File already exists!\n");
                terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            } else {
                terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
                terminal_writestring("Failed to create file\n");
                terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            }
        }
    } else if (shell_strcmp(cmd_args[0], "delete") == 0) {
        if (cmd_argc < 2) {
            terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
            terminal_writestring("Usage: delete <filename>\n");
            terminal_writestring("Example: delete test.txt\n");
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        } else {
            terminal_setcolor(vga_entry_color(VGA_COLOR_CYAN, VGA_COLOR_BLACK));
            terminal_writestring("Deleting file: ");
            terminal_writestring(cmd_args[1]);
            terminal_writestring("\n");
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            
            int result = memfs_simple_delete(cmd_args[1]);
            if (result == MEMFS_SUCCESS) {
                terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
                terminal_writestring("File deleted successfully!\n");
                terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            } else if (result == MEMFS_NOT_FOUND) {
                terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
                terminal_writestring("File not found!\n");
                terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            } else {
                terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
                terminal_writestring("Failed to delete file\n");
                terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            }
        }
    } else if (shell_strcmp(cmd_args[0], "write") == 0) {
        if (cmd_argc < 3) {
            terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
            terminal_writestring("Usage: write <filename> <text>\n");
            terminal_writestring("Example: write myfile.txt Hello World\n");
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        } else {
            // Combine all arguments after filename into content
            char content[256] = {0};
            int content_pos = 0;
            for (int i = 2; i < cmd_argc && content_pos < 250; i++) {
                if (i > 2) {
                    content[content_pos++] = ' ';
                }
                for (int j = 0; cmd_args[i][j] && content_pos < 250; j++) {
                    content[content_pos++] = cmd_args[i][j];
                }
            }
            content[content_pos] = '\0';
            
            terminal_setcolor(vga_entry_color(VGA_COLOR_CYAN, VGA_COLOR_BLACK));
            terminal_writestring("Writing to file: ");
            terminal_writestring(cmd_args[1]);
            terminal_writestring("\n");
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            
            int result = memfs_simple_write(cmd_args[1], content);
            if (result > 0) {
                terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
                terminal_writestring("Content written successfully!\n");
                terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            } else {
                terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
                terminal_writestring("Failed to write to file\n");
                terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            }
        }
    } else if (shell_strcmp(cmd_args[0], "fsinfo") == 0) {
        memfs_simple_dump_stats();
    } else if (cmd_argc > 0) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
        terminal_writestring("Command not found: ");
        terminal_writestring(cmd_args[0]);
        terminal_writestring("\n");
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    }
}

// Main kernel entry point
void kernel_main(void) {
    // Initialize terminal
    terminal_initialize();
    
    // Display welcome message
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("ClaudeOS Day 10 - Advanced Shell System\n");
    terminal_writestring("========================================\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Day 10: PMM + Syscalls + MemFS + Advanced Shell\n\n");
    
    // Initialize basic systems
    terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
    terminal_writestring("Initializing systems...\n");
    
    gdt_init();
    terminal_writestring("GDT: OK\n");
    
    idt_init();
    terminal_writestring("IDT: OK\n");
    
    pic_init();
    terminal_writestring("PIC: OK\n");
    
    timer_init();
    terminal_writestring("Timer: OK\n");
    
    keyboard_init();
    terminal_writestring("Keyboard: OK\n");
    
    if (serial_init(SERIAL_COM1_BASE) == 0) {
        terminal_writestring("Serial: OK\n");
    }
    
    pmm_init();
    terminal_writestring("PMM: OK\n");
    
    syscall_simple_init();
    terminal_writestring("Syscalls: OK\n");
    
    memfs_simple_init();
    terminal_writestring("MemFS: OK\n");
    
    // Enable interrupts
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Enabling interrupts...\n");
    asm volatile ("sti");
    terminal_writestring("All systems ready!\n\n");
    
    // Start shell
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    terminal_writestring("Type 'help' for available commands.\n\n");
    shell_print_prompt();
    
    // Main shell loop
    while (1) {
        asm volatile ("hlt");
        
        char c = keyboard_get_char();
        if (c != 0) {
            if (c == '\n') {
                terminal_putchar('\n');
                shell_buffer[shell_pos] = '\0';
                
                if (shell_pos > 0) {
                    shell_process_command(shell_buffer);
                }
                
                shell_pos = 0;
                shell_buffer[0] = '\0';
                shell_print_prompt();
                
            } else if (c == '\b') {
                if (shell_pos > 0) {
                    shell_pos--;
                    shell_buffer[shell_pos] = '\0';
                    terminal_putchar('\b');
                }
            } else if (c >= 32 && c <= 126) {
                if (shell_pos < 255) {
                    shell_buffer[shell_pos] = c;
                    shell_pos++;
                    terminal_putchar(c);
                }
            }
        }
    }
}