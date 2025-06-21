// ClaudeOS Demo Kernel - Stable Day 6 Base + Basic Shell
// Simplified version for demonstration without complex memory management

#include "kernel.h"
#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "timer.h"
#include "keyboard.h"
#include "serial.h"

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

// Simple shell state
static char shell_buffer[256];
static int shell_pos = 0;

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
        return;
    }
    
    if (c == '\b') {
        if (terminal_column > 0) {
            terminal_column--;
            terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
        }
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
}

// Simple shell functions
void shell_print_prompt(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("claudeos> ");
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
}

void shell_process_command(const char* cmd) {
    if (cmd[0] == 'h' && cmd[1] == 'e' && cmd[2] == 'l' && cmd[3] == 'p' && cmd[4] == '\0') {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
        terminal_writestring("ClaudeOS Demo Shell - Available Commands:\n");
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        terminal_writestring("  help     - Show this help\n");
        terminal_writestring("  clear    - Clear screen\n");
        terminal_writestring("  version  - Show version\n");
        terminal_writestring("  hello    - Say hello\n");
        terminal_writestring("  demo     - Demo message\n\n");
    } else if (cmd[0] == 'c' && cmd[1] == 'l' && cmd[2] == 'e' && cmd[3] == 'a' && cmd[4] == 'r' && cmd[5] == '\0') {
        terminal_clear();
        shell_print_prompt();
        return;
    } else if (cmd[0] == 'v' && cmd[1] == 'e' && cmd[2] == 'r') {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
        terminal_writestring("ClaudeOS Demo v1.0 - Day 11 Stable Shell\n");
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        terminal_writestring("Simplified version for demonstration\n");
    } else if (cmd[0] == 'h' && cmd[1] == 'e' && cmd[2] == 'l') {
        terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
        terminal_writestring("Hello from ClaudeOS Shell!\n");
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    } else if (cmd[0] == 'd' && cmd[1] == 'e' && cmd[2] == 'm') {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_MAGENTA, VGA_COLOR_BLACK));
        terminal_writestring("Demo: Interactive shell working!\n");
        terminal_writestring("Keyboard input processing successful!\n");
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    } else if (cmd[0] != '\0') {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
        terminal_writestring("Command not found: ");
        terminal_writestring(cmd);
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
    terminal_writestring("ClaudeOS Demo Shell - Day 11\n");
    terminal_writestring("=============================\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Stable demonstration version\n\n");
    
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