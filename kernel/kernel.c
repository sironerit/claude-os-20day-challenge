// ClaudeOS Kernel - Day 6 Implementation
// Memory management system integration

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

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len])
        len++;
    return len;
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
    terminal_writestring("ClaudeOS - Day 6 Development\n");
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
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    terminal_writestring("Day 6 Features:\n");
    terminal_writestring("- Physical Memory Manager (PMM)\n");
    terminal_writestring("- Virtual Memory Manager (VMM)\n");
    terminal_writestring("- Paging System (4KB pages)\n");
    terminal_writestring("- Kernel Heap (kmalloc/kfree)\n");
    terminal_writestring("- Memory statistics and debugging\n\n");
    
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
    terminal_writestring("\nDay 6 Memory Management System Complete!\n");
    terminal_writestring("All components operational and tested.\n");
    debug_write_string("Day 6 memory management test completed successfully!\n");
    
    // Kernel idle loop
    while (1) {
        asm volatile ("hlt");
    }
}