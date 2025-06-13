#include "serial.h"

// I/O port access functions (from kernel/io.asm)
extern void outb(uint16_t port, uint8_t data);
extern uint8_t inb(uint16_t port);

/**
 * Initialize serial port
 * @param port Base port address (e.g., COM1_PORT)
 * @param baud_divisor Baud rate divisor (e.g., BAUD_115200)
 */
void serial_init(uint16_t port, uint16_t baud_divisor) {
    // Disable all interrupts
    outb(port + UART_INT_ENABLE, 0x00);
    
    // Enable DLAB (set baud rate divisor)
    outb(port + UART_LINE_CTRL, UART_LCR_DIVISOR_LATCH);
    
    // Set divisor (low byte)
    outb(port + UART_DATA, (uint8_t)(baud_divisor & 0xFF));
    
    // Set divisor (high byte)
    outb(port + UART_INT_ENABLE, (uint8_t)((baud_divisor >> 8) & 0xFF));
    
    // Configure line: 8 bits, no parity, one stop bit
    outb(port + UART_LINE_CTRL, UART_LCR_WORD_LENGTH_8);
    
    // Enable FIFO, clear them, with 14-byte threshold
    outb(port + UART_FIFO_CTRL, 
         UART_FCR_ENABLE_FIFO | 
         UART_FCR_CLEAR_RECV_FIFO | 
         UART_FCR_CLEAR_SEND_FIFO | 
         UART_FCR_TRIGGER_14);
    
    // Enable DTR, RTS, and OUT2 (required for interrupts)
    outb(port + UART_MODEM_CTRL, 
         UART_MCR_DTR | 
         UART_MCR_RTS | 
         UART_MCR_OUT2);
}

/**
 * Send a character via serial port
 */
void serial_putchar(uint16_t port, char c) {
    // Wait for transmit buffer to be empty
    while ((inb(port + UART_LINE_STATUS) & UART_LSR_TRANSMIT_EMPTY) == 0) {
        // Busy wait
    }
    
    // Send character
    outb(port + UART_DATA, (uint8_t)c);
}

/**
 * Send a string via serial port
 */
void serial_puts(uint16_t port, const char* str) {
    while (*str) {
        serial_putchar(port, *str);
        str++;
    }
}

/**
 * Receive a character from serial port
 * Returns 0 if no data available
 */
char serial_getchar(uint16_t port) {
    // Check if data is available
    if ((inb(port + UART_LINE_STATUS) & UART_LSR_DATA_READY) == 0) {
        return 0;  // No data available
    }
    
    // Read and return character
    return (char)inb(port + UART_DATA);
}

/**
 * Check if serial data is available
 */
int serial_available(uint16_t port) {
    return (inb(port + UART_LINE_STATUS) & UART_LSR_DATA_READY) != 0;
}

/**
 * Simple printf implementation for serial output
 * Supports %s (string), %c (character), %d (decimal), %x (hex)
 */
void serial_printf(uint16_t port, const char* format, ...) {
    // Simple implementation - will expand this later
    // For now, just output the format string
    serial_puts(port, format);
}

// Convenience functions for COM1 debug output

/**
 * Initialize COM1 for debug output at 115200 baud
 */
void serial_debug_init(void) {
    serial_init(COM1_PORT, BAUD_115200);
}

/**
 * Send debug character to COM1
 */
void serial_debug_putchar(char c) {
    serial_putchar(COM1_PORT, c);
}

/**
 * Send debug string to COM1
 */
void serial_debug_puts(const char* str) {
    serial_puts(COM1_PORT, str);
}

/**
 * Debug printf to COM1
 */
void serial_debug_printf(const char* format, ...) {
    serial_printf(COM1_PORT, format);
}

/**
 * Convert integer to string (helper function)
 */
__attribute__((unused))
static void itoa(int value, char* str, int base) {
    char* ptr = str;
    char* ptr1 = str;
    char tmp_char;
    int tmp_value;
    
    // Handle negative numbers for decimal
    if (value < 0 && base == 10) {
        *ptr++ = '-';
        value = -value;
        ptr1++;
    }
    
    // Convert to string (reverse order)
    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "0123456789abcdef"[tmp_value - value * base];
    } while (value);
    
    *ptr-- = '\0';
    
    // Reverse string
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
}