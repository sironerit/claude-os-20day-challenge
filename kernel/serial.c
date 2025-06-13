// ClaudeOS Serial Port Implementation - Day 5
// UART 16550 serial communication driver

#include "serial.h"
#include "pic.h"
#include "kernel.h"

// Initialize serial port
int serial_init(uint16_t port) {
    // Disable interrupts
    outb(port + SERIAL_INT_ENABLE_REG, 0x00);
    
    // Set DLAB to configure baud rate
    outb(port + SERIAL_LINE_CTRL_REG, SERIAL_LCR_DLAB);
    
    // Set baud rate to 9600 (divisor = 12)
    outb(port + SERIAL_DIVISOR_LOW, BAUD_9600 & 0xFF);
    outb(port + SERIAL_DIVISOR_HIGH, (BAUD_9600 >> 8) & 0xFF);
    
    // Configure line: 8 bits, no parity, 1 stop bit
    outb(port + SERIAL_LINE_CTRL_REG, SERIAL_LCR_8BITS | SERIAL_LCR_1STOP | SERIAL_LCR_NO_PARITY);
    
    // Enable FIFO with 14-byte trigger level
    outb(port + SERIAL_FIFO_CTRL_REG, SERIAL_FCR_ENABLE | SERIAL_FCR_CLEAR_RX | 
         SERIAL_FCR_CLEAR_TX | SERIAL_FCR_TRIGGER_14);
    
    // Enable DTR, RTS, and IRQ
    outb(port + SERIAL_MODEM_CTRL_REG, SERIAL_MCR_DTR | SERIAL_MCR_RTS | SERIAL_MCR_OUT2);
    
    // Test serial port with loopback
    outb(port + SERIAL_MODEM_CTRL_REG, SERIAL_MCR_DTR | SERIAL_MCR_RTS | 
         SERIAL_MCR_OUT1 | SERIAL_MCR_OUT2 | SERIAL_MCR_LOOPBACK);
    
    // Send test byte
    outb(port + SERIAL_DATA_REG, 0xAE);
    
    // Check if we get the same byte back
    if (inb(port + SERIAL_DATA_REG) != 0xAE) {
        return -1;  // Serial port failed test
    }
    
    // Disable loopback and enable normal operation
    outb(port + SERIAL_MODEM_CTRL_REG, SERIAL_MCR_DTR | SERIAL_MCR_RTS | SERIAL_MCR_OUT2);
    
    return 0;  // Success
}

// Check if data is available to read
int serial_received(uint16_t port) {
    return inb(port + SERIAL_LINE_STATUS_REG) & SERIAL_LSR_DATA_READY;
}

// Read a character from serial port
char serial_getchar(uint16_t port) {
    while (!serial_received(port)) {
        // Wait for data
    }
    return inb(port + SERIAL_DATA_REG);
}

// Check if transmit buffer is empty
int serial_is_transmit_empty(uint16_t port) {
    return inb(port + SERIAL_LINE_STATUS_REG) & SERIAL_LSR_TX_EMPTY;
}

// Send a character to serial port
void serial_putchar(uint16_t port, char c) {
    while (!serial_is_transmit_empty(port)) {
        // Wait for transmit buffer to be empty
    }
    outb(port + SERIAL_DATA_REG, c);
}

// Send a string to serial port
void serial_write_string(uint16_t port, const char* str) {
    while (*str) {
        serial_putchar(port, *str);
        str++;
    }
}

// Debug output functions (using COM1)
void debug_putchar(char c) {
    serial_putchar(SERIAL_COM1_BASE, c);
}

void debug_write_string(const char* str) {
    serial_write_string(SERIAL_COM1_BASE, str);
}