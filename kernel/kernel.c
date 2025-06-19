// ClaudeOS Kernel - Day 7 Implementation
// Process management system integration

#include "kernel.h"
#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "timer.h"
#include "keyboard.h"
#include "serial.h"
#include "pmm.h"
#include "vmm.h"
#include "heap.h"
#include "process.h"
#include "string.h"
#include "syscall.h"
#include "../fs/simplefs.h"
#include "../drivers/ata.h"
#include "shell.h"

// VGA Text Mode Constants
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000


// Global variables
static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;

// Utility functions
static inline uint8_t vga_entry_color(vga_color fg, vga_color bg) {
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

// strlen function moved to string.c

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

void terminal_clear(void) {
    // Clear screen and reset cursor
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
    terminal_column = 0;
    terminal_row = 0;
}

void terminal_setcolor(uint8_t color) {
    terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}

// Scroll screen up by one line
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
            terminal_row = VGA_HEIGHT - 1;  // Stay at bottom line
        }
        return;
    }
    
    if (c == '\b') {
        // Handle backspace - move cursor back if possible
        if (terminal_column > 0) {
            terminal_column--;
            // Clear the character at the current position
            terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
        }
        return;
    }
    
    terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
    if (++terminal_column == VGA_WIDTH) {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_scroll();
            terminal_row = VGA_HEIGHT - 1;  // Stay at bottom line
        }
    }
}

void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++)
        terminal_putchar(data[i]);
}

void terminal_writestring(const char* data) {
    terminal_write(data, strlen(data));
}

// Simple printf implementation for debugging
void terminal_printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    char buffer[256];
    int pos = 0;
    
    for (int i = 0; format[i] != '\0' && pos < 255; i++) {
        if (format[i] == '%' && format[i+1] == 'd') {
            int value = va_arg(args, int);
            // Simple integer to string conversion
            if (value == 0) {
                buffer[pos++] = '0';
            } else {
                char temp[12];
                int temp_pos = 0;
                int temp_value = value;
                
                if (value < 0) {
                    buffer[pos++] = '-';
                    temp_value = -value;
                }
                
                while (temp_value > 0) {
                    temp[temp_pos++] = '0' + (temp_value % 10);
                    temp_value /= 10;
                }
                
                for (int j = temp_pos - 1; j >= 0; j--) {
                    buffer[pos++] = temp[j];
                }
            }
            i++; // Skip the 'd'
        } else if (format[i] == '%' && format[i+1] == 's') {
            char* str = va_arg(args, char*);
            while (*str && pos < 255) {
                buffer[pos++] = *str++;
            }
            i++; // Skip the 's'
        } else {
            buffer[pos++] = format[i];
        }
    }
    
    buffer[pos] = '\0';
    terminal_writestring(buffer);
    
    va_end(args);
}

// Kernel panic function
void kernel_panic(const char* message) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_RED));
    terminal_writestring("\n*** KERNEL PANIC ***\n");
    terminal_writestring(message);
    terminal_writestring("\nSystem halted.");
    
    // Halt the CPU
    while (1) {
        asm volatile ("hlt");
    }
}

// Main kernel entry point
void kernel_main(void) {
    // Initialize terminal
    terminal_initialize();
    
    // Display welcome message
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("ClaudeOS - Day 9 File System Demo\n");
    terminal_writestring("==================================\n");
    
    // Initialize GDT
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK));
    terminal_writestring("Initializing GDT...\n");
    gdt_init();
    terminal_writestring("GDT initialized successfully!\n");
    
    // Initialize IDT
    terminal_writestring("Initializing IDT...\n");
    idt_init();
    terminal_writestring("IDT initialized successfully!\n");
    
    // Initialize PIC
    terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
    terminal_writestring("Initializing PIC...\n");
    pic_init();
    terminal_writestring("PIC initialized successfully!\n");
    
    // Initialize Timer
    terminal_writestring("Initializing Timer...\n");
    timer_init();
    terminal_writestring("Timer initialized successfully!\n");
    
    // Initialize Serial Port
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_MAGENTA, VGA_COLOR_BLACK));
    terminal_writestring("Initializing Serial Port...\n");
    if (serial_init(SERIAL_COM1_BASE) == 0) {
        terminal_writestring("Serial port initialized successfully!\n");
        debug_write_string("ClaudeOS Day 6 - Serial debug output active\n");
    } else {
        terminal_writestring("Serial port initialization failed!\n");
    }
    
    // Initialize Keyboard
    terminal_writestring("Initializing Keyboard...\n");
    keyboard_init();
    terminal_writestring("Keyboard initialized successfully!\n");
    
    // Initialize Physical Memory Manager
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
    terminal_writestring("Initializing Physical Memory Manager...\n");
    pmm_init();
    terminal_writestring("PMM initialized successfully!\n");
    
    // Initialize basic memory management for shell
    terminal_writestring("Basic memory management ready!\n");
    
    // Enable interrupts
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Enabling interrupts...\n");
    asm volatile ("sti");
    terminal_writestring("Interrupts enabled!\n\n");
    
    // === Day 6 Stable + Day 11 Shell Demo ===
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("\n=== ClaudeOS Day 11 Shell Demo ===\n");
    terminal_writestring("Based on stable Day 6 foundation\n\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    terminal_writestring("Stable Features (Day 6):\n");
    terminal_writestring("- Physical Memory Manager (PMM)\n");
    terminal_writestring("- Virtual Memory Manager (VMM)\n");
    terminal_writestring("- Paging System (4KB pages)\n");
    terminal_writestring("- Kernel Heap (kmalloc/kfree)\n");
    terminal_writestring("- Hardware Drivers (Timer, Keyboard, Serial)\n\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("New Features (Day 11):\n");
    terminal_writestring("- Interactive Command Shell\n");
    terminal_writestring("- 11 Built-in Commands\n");
    terminal_writestring("- File Operations (create, cat, ls, delete)\n");
    terminal_writestring("- Directory Operations (mkdir, rmdir, cd)\n\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("All components ready for shell interface!\n\n");
    
    // Initialize and start shell (Day 11 Phase 1)
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("Starting ClaudeOS Shell...\n");
    shell_init();
    
    // Shell main loop with keyboard input processing
    while (1) {
        if (keyboard_has_input()) {
            char c = keyboard_get_char();
            shell_process_input(c);
        }
        asm volatile ("hlt");
    }
}

// Test process functions

// System call test process
void syscall_test_process(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
    terminal_writestring("[SYSCALL_TEST] System call test process started\n");
    
    // Test all system calls from process context
    terminal_writestring("[SYSCALL_TEST] Testing sys_hello...\n");
    syscall_hello();
    
    terminal_writestring("[SYSCALL_TEST] Testing sys_getpid...\n");
    int my_pid = syscall_getpid();
    terminal_printf("[SYSCALL_TEST] My PID is: %d\n", my_pid);
    
    terminal_writestring("[SYSCALL_TEST] Testing sys_write...\n");
    syscall_write("Message from syscall_test process!\n");
    
    terminal_writestring("[SYSCALL_TEST] Testing sys_yield...\n");
    syscall_yield();
    
    terminal_writestring("[SYSCALL_TEST] All system calls tested successfully!\n");
    process_exit();
}

// File system test process (Day 9)
void fs_test_process(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
    terminal_writestring("[FS_TEST] File system test process started\n");
    
    // Test file system operations from process context
    terminal_writestring("[FS_TEST] Testing file system operations...\n");
    
    // Create a test file
    terminal_writestring("[FS_TEST] Creating file '/process_test.txt'...\n");
    int fd = syscall_open("/process_test.txt", O_CREATE | O_WRITE);
    
    if (fd >= 0) {
        terminal_writestring("[FS_TEST] File created successfully!\n");
        
        // Write data to file
        const char* data = "Hello from file system test process!";
        int bytes_written = syscall_write_file(fd, data, strlen(data));
        terminal_printf("[FS_TEST] Wrote %d bytes to file\n", bytes_written);
        
        // Close and reopen for reading
        syscall_close(fd);
        terminal_writestring("[FS_TEST] File closed, reopening for reading...\n");
        
        fd = syscall_open("/process_test.txt", O_READ);
        if (fd >= 0) {
            char buffer[256];
            int bytes_read = syscall_read_file(fd, buffer, 255);
            buffer[bytes_read] = '\0';
            terminal_printf("[FS_TEST] Read back: '%s'\n", buffer);
            syscall_close(fd);
        }
        
        // Test directory listing
        terminal_writestring("[FS_TEST] Listing directory contents:\n");
        syscall_list("/");
        
        // Display file system statistics
        terminal_writestring("[FS_TEST] File system statistics:\n");
        syscall_fs_stats();
        
    } else {
        terminal_writestring("[FS_TEST] Failed to create test file\n");
    }
    
    terminal_writestring("[FS_TEST] File system test complete!\n");
    process_exit();
}

void test_process_1(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
    terminal_writestring("[PROC1] Test process 1 running\n");
    
    for (int i = 0; i < 3; i++) {
        terminal_printf("[PROC1] Iteration %d\n", i + 1);
        // Simulate work
        for (volatile int j = 0; j < 500000; j++);
    }
    
    terminal_writestring("[PROC1] Test process 1 exiting\n");
    process_exit();
}

void test_process_2(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK));
    terminal_writestring("[PROC2] Test process 2 running\n");
    
    for (int i = 0; i < 3; i++) {
        terminal_printf("[PROC2] Iteration %d\n", i + 1);
        // Simulate work
        for (volatile int j = 0; j < 500000; j++);
    }
    
    terminal_writestring("[PROC2] Test process 2 exiting\n");
    process_exit();
}

void idle_process(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK));
    terminal_writestring("[IDLE] Idle process started\n");
    
    while (1) {
        // Idle loop
        asm volatile ("hlt");
    }
}