#ifndef SERIAL_H
#define SERIAL_H

#include "types.h"

// Serial port constants
#define SERIAL_COM1_BASE    0x3F8
#define SERIAL_COM2_BASE    0x2F8
#define SERIAL_COM3_BASE    0x3E8
#define SERIAL_COM4_BASE    0x2E8

// Serial port register offsets
#define SERIAL_DATA_REG         0  // Data register (read/write)
#define SERIAL_INT_ENABLE_REG   1  // Interrupt enable register
#define SERIAL_FIFO_CTRL_REG    2  // FIFO control register
#define SERIAL_LINE_CTRL_REG    3  // Line control register
#define SERIAL_MODEM_CTRL_REG   4  // Modem control register
#define SERIAL_LINE_STATUS_REG  5  // Line status register
#define SERIAL_MODEM_STATUS_REG 6  // Modem status register

// Baud rate divisor registers (when DLAB=1)
#define SERIAL_DIVISOR_LOW      0  // Low byte of divisor
#define SERIAL_DIVISOR_HIGH     1  // High byte of divisor

// Line Control Register bits
#define SERIAL_LCR_5BITS        0x00  // 5 data bits
#define SERIAL_LCR_6BITS        0x01  // 6 data bits
#define SERIAL_LCR_7BITS        0x02  // 7 data bits
#define SERIAL_LCR_8BITS        0x03  // 8 data bits
#define SERIAL_LCR_1STOP        0x00  // 1 stop bit
#define SERIAL_LCR_2STOP        0x04  // 2 stop bits
#define SERIAL_LCR_NO_PARITY    0x00  // No parity
#define SERIAL_LCR_ODD_PARITY   0x08  // Odd parity
#define SERIAL_LCR_EVEN_PARITY  0x18  // Even parity
#define SERIAL_LCR_DLAB         0x80  // Divisor Latch Access Bit

// Line Status Register bits
#define SERIAL_LSR_DATA_READY   0x01  // Data ready
#define SERIAL_LSR_OVERRUN      0x02  // Overrun error
#define SERIAL_LSR_PARITY_ERR   0x04  // Parity error
#define SERIAL_LSR_FRAME_ERR    0x08  // Framing error
#define SERIAL_LSR_BREAK        0x10  // Break interrupt
#define SERIAL_LSR_TX_EMPTY     0x20  // Transmitter holding register empty
#define SERIAL_LSR_TX_SHIFT     0x40  // Transmitter empty
#define SERIAL_LSR_FIFO_ERR     0x80  // Error in FIFO

// FIFO Control Register bits
#define SERIAL_FCR_ENABLE       0x01  // Enable FIFO
#define SERIAL_FCR_CLEAR_RX     0x02  // Clear receive FIFO
#define SERIAL_FCR_CLEAR_TX     0x04  // Clear transmit FIFO
#define SERIAL_FCR_TRIGGER_1    0x00  // 1-byte trigger level
#define SERIAL_FCR_TRIGGER_4    0x40  // 4-byte trigger level
#define SERIAL_FCR_TRIGGER_8    0x80  // 8-byte trigger level
#define SERIAL_FCR_TRIGGER_14   0xC0  // 14-byte trigger level

// Modem Control Register bits
#define SERIAL_MCR_DTR          0x01  // Data Terminal Ready
#define SERIAL_MCR_RTS          0x02  // Request To Send
#define SERIAL_MCR_OUT1         0x04  // Output 1
#define SERIAL_MCR_OUT2         0x08  // Output 2 (enables interrupts)
#define SERIAL_MCR_LOOPBACK     0x10  // Loopback mode

// Common baud rates (divisors for 115200 base)
#define BAUD_115200             1
#define BAUD_57600              2
#define BAUD_38400              3
#define BAUD_19200              6
#define BAUD_9600               12
#define BAUD_4800               24
#define BAUD_2400               48
#define BAUD_1200               96

// Function declarations
int serial_init(uint16_t port);
void serial_putchar(uint16_t port, char c);
void serial_write_string(uint16_t port, const char* str);
char serial_getchar(uint16_t port);
int serial_received(uint16_t port);
int serial_is_transmit_empty(uint16_t port);

// Debug output functions (using COM1)
void debug_putchar(char c);
void debug_write_string(const char* str);

#endif // SERIAL_H