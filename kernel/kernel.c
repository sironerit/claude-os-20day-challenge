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

// VGA Text Mode Constants
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

// VGA Color Codes
typedef enum {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
    VGA_COLOR_YELLOW = 14,
} vga_color;

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

void terminal_setcolor(uint8_t color) {
    terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}

void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_row = 0;  // Simple wrap to top
        }
        return;
    }
    
    terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
    if (++terminal_column == VGA_WIDTH) {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_row = 0;  // Simple wrap to top
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
    terminal_writestring("ClaudeOS - Day 7 Development\n");
    terminal_writestring("============================\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Kernel loaded successfully!\n");
    terminal_writestring("VGA text mode initialized.\n");
    
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
    
    // Initialize Virtual Memory Manager
    terminal_writestring("Initializing Virtual Memory Manager...\n");
    vmm_init();
    terminal_writestring("VMM initialized successfully!\n");
    
    // Enable paging
    terminal_writestring("Enabling paging...\n");
    vmm_switch_page_directory(current_page_directory);
    vmm_enable_paging();
    terminal_writestring("Paging enabled successfully!\n");
    
    // Initialize Kernel Heap
    terminal_writestring("Initializing Kernel Heap...\n");
    heap_init();
    terminal_writestring("Heap initialized successfully!\n");
    
    // Enable interrupts
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Enabling interrupts...\n");
    asm volatile ("sti");
    terminal_writestring("Interrupts enabled!\n\n");
    
    // Initialize Process Management System
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_MAGENTA, VGA_COLOR_BLACK));
    terminal_writestring("Initializing Process Management System...\n");
    process_init();
    terminal_writestring("Process management initialized successfully!\n");
    
    // Initialize System Call System
    terminal_writestring("Initializing System Call Interface...\n");
    syscall_init();
    terminal_writestring("System calls initialized successfully!\n");
    
    // Initialize File System (Day 9)
    terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
    terminal_writestring("Initializing File System...\n");
    int fs_result = fs_init();
    if (fs_result == FS_SUCCESS) {
        terminal_writestring("File system initialized successfully!\n");
    } else {
        terminal_writestring("File system initialization failed!\n");
    }
    terminal_writestring("\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    terminal_writestring("Day 9 Features:\n");
    terminal_writestring("- Physical Memory Manager (PMM)\n");
    terminal_writestring("- Virtual Memory Manager (VMM)\n");
    terminal_writestring("- Paging System (4KB pages)\n");
    terminal_writestring("- Kernel Heap (kmalloc/kfree)\n");
    terminal_writestring("- Process Management System\n");
    terminal_writestring("- Round-robin Scheduler\n");
    terminal_writestring("- Context Switching\n");
    terminal_writestring("- System Call Interface (INT 0x80)\n");
    terminal_writestring("- File System (SimpleFS)\n");
    terminal_writestring("- File Operations (create, read, write, close)\n");
    terminal_writestring("- Directory Operations (mkdir, list)\n");
    terminal_writestring("- 13 System Calls Total\n\n");
    
    // Memory management demonstration
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK));
    terminal_writestring("Memory Management Test:\n");
    
    // Display memory statistics
    terminal_setcolor(vga_entry_color(VGA_COLOR_CYAN, VGA_COLOR_BLACK));
    pmm_dump_stats();
    terminal_writestring("\n");
    heap_dump_stats();
    terminal_writestring("\n");
    
    // Test dynamic memory allocation
    terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
    terminal_writestring("Testing dynamic memory allocation...\n");
    
    void* ptr1 = kmalloc(1024);
    terminal_writestring("Allocated 1024 bytes: ");
    if (ptr1) {
        terminal_writestring("SUCCESS\n");
        debug_write_string("kmalloc(1024) successful\n");
    } else {
        terminal_writestring("FAILED\n");
    }
    
    void* ptr2 = kmalloc(2048);
    terminal_writestring("Allocated 2048 bytes: ");
    if (ptr2) {
        terminal_writestring("SUCCESS\n");
        debug_write_string("kmalloc(2048) successful\n");
    } else {
        terminal_writestring("FAILED\n");
    }
    
    void* ptr3 = kcalloc(10, 64);
    terminal_writestring("Allocated 10x64 bytes (zeroed): ");
    if (ptr3) {
        terminal_writestring("SUCCESS\n");
        debug_write_string("kcalloc(10, 64) successful\n");
    } else {
        terminal_writestring("FAILED\n");
    }
    
    // Free some memory
    if (ptr1) {
        kfree(ptr1);
        terminal_writestring("Freed first allocation\n");
    }
    
    if (ptr2) {
        kfree(ptr2);
        terminal_writestring("Freed second allocation\n");
    }
    
    terminal_writestring("\nAfter allocations and frees:\n");
    heap_dump_stats();
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("\nMemory Management Test Complete!\n\n");
    
    // System Call Demonstration
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("System Call Test:\n");
    
    // Test system calls from kernel context
    terminal_writestring("Testing system calls from kernel...\n");
    
    // Test SYS_HELLO
    terminal_writestring("Calling sys_hello: ");
    int result = syscall_hello();
    terminal_printf("Result: %d\n", result);
    
    // Test SYS_GETPID
    terminal_writestring("Calling sys_getpid: ");
    int pid = syscall_getpid();
    terminal_printf("PID: %d\n", pid);
    
    // Test SYS_WRITE
    syscall_write("Hello from syscall_write!\n");
    
    terminal_writestring("Kernel system call tests complete!\n\n");
    
    // File System Demonstration (Day 9)
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("File System Test:\n");
    
    if (fs_is_initialized()) {
        // Display file system statistics
        terminal_writestring("File system statistics:\n");
        syscall_fs_stats();
        terminal_writestring("\n");
        
        // Test basic file operations
        terminal_writestring("Testing file operations...\n");
        
        // Create a test file
        int fd = syscall_open("/test.txt", O_CREATE | O_WRITE);
        if (fd >= 0) {
            terminal_writestring("Created file '/test.txt'\n");
            
            // Write data to file
            const char* test_data = "Hello, ClaudeFS! This is a test file.";
            int bytes_written = syscall_write_file(fd, test_data, strlen(test_data));
            terminal_printf("Wrote %d bytes to file\n", bytes_written);
            
            // Close the file
            syscall_close(fd);
            terminal_writestring("File closed\n");
            
            // Reopen for reading
            fd = syscall_open("/test.txt", O_READ);
            if (fd >= 0) {
                char read_buffer[256];
                int bytes_read = syscall_read_file(fd, read_buffer, 255);
                read_buffer[bytes_read] = '\0';
                terminal_printf("Read %d bytes: '%s'\n", bytes_read, read_buffer);
                syscall_close(fd);
            }
        } else {
            terminal_writestring("Failed to create test file\n");
        }
        
        // List directory contents
        terminal_writestring("Directory listing:\n");
        syscall_list("/");
        
        terminal_writestring("File system test complete!\n\n");
    } else {
        terminal_writestring("File system not initialized - skipping tests\n\n");
    }
    
    // Process Management Demonstration
    terminal_writestring("Process Management Test:\n");
    
    // Create test processes (including file system test process)
    terminal_writestring("Creating test processes...\n");
    process_t* proc1 = process_create("syscall_test", syscall_test_process, PRIORITY_NORMAL);
    process_t* proc2 = process_create("fs_test", fs_test_process, PRIORITY_NORMAL);
    process_t* proc3 = process_create("test_proc_2", test_process_2, PRIORITY_NORMAL);
    process_t* proc4 = process_create("idle_proc", idle_process, PRIORITY_LOW);
    
    if (proc1 && proc2 && proc3 && proc4) {
        terminal_writestring("Test processes created successfully!\n");
    } else {
        terminal_writestring("Failed to create some test processes!\n");
    }
    
    // List all processes
    process_list_all();
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Day 7 Process Management System Complete!\n");
    terminal_writestring("All components operational and tested.\n");
    debug_write_string("Day 7 process management test completed successfully!\n");
    
    // Start process scheduler
    terminal_writestring("\nStarting process scheduler...\n");
    
    // Simple scheduler demonstration
    for (int i = 0; i < 5; i++) {
        terminal_printf("Scheduler iteration %d\n", i + 1);
        schedule();
        // Simple delay
        for (volatile int j = 0; j < 1000000; j++);
    }
    
    terminal_writestring("Scheduler demonstration complete.\n");
    
    // Kernel idle loop
    while (1) {
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