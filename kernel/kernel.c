// ClaudeOS Kernel - Day 4 Implementation
// Hardware drivers integration

#include "kernel.h"
#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "timer.h"

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
    VGA_COLOR_LIGHT_YELLOW = 14,
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
    terminal_writestring("ClaudeOS - Day 4 Development\n");
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
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_YELLOW, VGA_COLOR_BLACK));
    terminal_writestring("Initializing PIC...\n");
    pic_init();
    terminal_writestring("PIC initialized successfully!\n");
    
    // Initialize Timer
    terminal_writestring("Initializing Timer...\n");
    timer_init();
    terminal_writestring("Timer initialized successfully!\n");
    
    // Enable interrupts
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Enabling interrupts...\n");
    asm volatile ("sti");
    terminal_writestring("Interrupts enabled!\n\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    terminal_writestring("Day 4 Features:\n");
    terminal_writestring("- GDT (Global Descriptor Table)\n");
    terminal_writestring("- IDT (Interrupt Descriptor Table)\n");
    terminal_writestring("- PIC (Programmable Interrupt Controller)\n");
    terminal_writestring("- Timer (100Hz system clock)\n");
    terminal_writestring("- Hardware interrupt support\n\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK));
    terminal_writestring("Status: Day 4 hardware drivers active\n");
    terminal_writestring("Timer ticking at 100Hz frequency\n\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
    terminal_writestring("Timer demonstration:\n");
    
    // Demonstrate timer functionality
    uint32_t start_ticks = timer_get_ticks();
    terminal_writestring("Waiting 2 seconds...\n");
    timer_wait(200);  // Wait 2 seconds (200 ticks at 100Hz)
    uint32_t end_ticks = timer_get_ticks();
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Timer test completed!\n");
    // Simple tick count display (just show if it's working)
    if (end_ticks > start_ticks) {
        terminal_writestring("Timer is working correctly\n");
    }
    
    // Kernel idle loop
    while (1) {
        asm volatile ("hlt");
    }
}