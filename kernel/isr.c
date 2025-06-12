#include "kernel.h"

// VGA Color definitions (copied from kernel.c)
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
} vga_color;

// VGA helper function
static inline uint8_t vga_entry_color(vga_color fg, vga_color bg) {
    return fg | bg << 4;
}

// ISR structure for passing information about the interrupt
typedef struct {
    uint32_t ds;                     // Data segment selector
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha
    uint32_t int_no, err_code;       // Interrupt number and error code (if applicable)
    uint32_t eip, cs, eflags, useresp, ss; // Pushed by the processor automatically
} registers_t;

// Exception messages
static const char* exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow", 
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception"
};

// ISR handler - called from assembly
void isr_handler(registers_t regs) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
    terminal_writestring("\n*** EXCEPTION CAUGHT ***\n");
    
    // Display exception information
    if (regs.int_no < 20) {
        terminal_writestring("Exception: ");
        terminal_writestring(exception_messages[regs.int_no]);
        terminal_writestring("\n");
    } else {
        terminal_writestring("Unknown exception\n");
    }
    
    // Display error code if present
    if (regs.err_code != 0) {
        terminal_writestring("Error code: ");
        // Simple hex display (for demonstration)
        terminal_writestring("0x????????\n");
    }
    
    terminal_writestring("System halted due to exception.\n");
    kernel_panic("Exception occurred");
}

// IRQ handler - called from assembly  
void irq_handler(registers_t regs) {
    // Send End Of Interrupt (EOI) signal to PICs
    if (regs.int_no >= 40) {
        // Send reset signal to slave
        outb(0xA0, 0x20);
    }
    // Send reset signal to master (always)
    outb(0x20, 0x20);
    
    switch (regs.int_no) {
        case 32: // Timer interrupt
            timer_callback();
            break;
        case 33: // Keyboard interrupt
            keyboard_callback();
            break;
        default:
            // Unknown IRQ
            break;
    }
}

// Timer callback (simple counter for demonstration)
static uint32_t timer_ticks = 0;

void timer_callback(void) {
    timer_ticks++;
    
    // Display timer every 100 ticks (roughly 1 second)
    if (timer_ticks % 100 == 0) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
        terminal_writestring("Timer: ");
        // Simple tick display
        terminal_writestring("tick\n");
    }
}

// Keyboard callback (basic key detection)
void keyboard_callback(void) {
    uint8_t scancode = inb(0x60); // Read from keyboard port
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK));
    terminal_writestring("Key pressed: ");
    // Simple scancode display
    terminal_writestring("key\n");
    
    (void)scancode; // Suppress unused variable warning
}