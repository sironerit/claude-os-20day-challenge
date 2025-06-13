// ClaudeOS Interrupt Service Routines - Day 3
// C implementation of exception handlers

#include "kernel.h"
#include "types.h"

// VGA Color Codes (for exception display)
typedef enum {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_WHITE = 15,
    VGA_COLOR_RED = 4,
} vga_color;

static inline uint8_t vga_entry_color(vga_color fg, vga_color bg) {
    return fg | bg << 4;
}

// Register structure for ISR context
struct registers {
    uint32_t ds;                                     // Data segment selector
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // General purpose registers
    uint32_t int_no, err_code;                       // Interrupt number and error code
    uint32_t eip, cs, eflags, useresp, ss;           // Pushed automatically by CPU
};

// Exception names for debugging
static const char* exception_names[] = {
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
    "Unknown Interrupt"
};

// ISR handler function
void isr_handler(struct registers regs) {
    // Get exception name
    const char* exception_name;
    if (regs.int_no < 15) {
        exception_name = exception_names[regs.int_no];
    } else {
        exception_name = exception_names[15]; // "Unknown Interrupt"
    }
    
    // Display exception information
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_RED));
    terminal_writestring("\n*** EXCEPTION OCCURRED ***\n");
    
    terminal_writestring("Exception: ");
    terminal_writestring(exception_name);
    terminal_writestring("\n");
    
    // Display interrupt number
    terminal_writestring("Interrupt Number: ");
    if (regs.int_no < 10) {
        char num_str[2] = {'0' + regs.int_no, '\0'};
        terminal_writestring(num_str);
    } else {
        terminal_writestring("10+");
    }
    terminal_writestring("\n");
    
    // Display error code if present
    if (regs.err_code != 0) {
        terminal_writestring("Error Code: ");
        terminal_writestring("(Present)\n");
    }
    
    terminal_writestring("System halted due to exception.\n");
    
    // Halt the system
    while (1) {
        asm volatile ("hlt");
    }
}