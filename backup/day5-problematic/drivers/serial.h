#ifndef SERIAL_H
#define SERIAL_H

#include "../kernel/types.h"

// Serial Port Driver (UART 16550)
// For debug output and communication

// Serial port base addresses
#define COM1_PORT   0x3F8
#define COM2_PORT   0x2F8
#define COM3_PORT   0x3E8
#define COM4_PORT   0x2E8

// UART register offsets
#define UART_DATA           0   // Data register (read/write)
#define UART_INT_ENABLE     1   // Interrupt enable register
#define UART_FIFO_CTRL      2   // FIFO control register
#define UART_LINE_CTRL      3   // Line control register
#define UART_MODEM_CTRL     4   // Modem control register
#define UART_LINE_STATUS    5   // Line status register
#define UART_MODEM_STATUS   6   // Modem status register
#define UART_SCRATCH        7   // Scratch register

// Line status register bits
#define UART_LSR_DATA_READY         0x01
#define UART_LSR_OVERRUN_ERROR      0x02
#define UART_LSR_PARITY_ERROR       0x04
#define UART_LSR_FRAMING_ERROR      0x08
#define UART_LSR_BREAK_INTERRUPT    0x10
#define UART_LSR_TRANSMIT_EMPTY     0x20
#define UART_LSR_TRANSMIT_SHIFT     0x40
#define UART_LSR_FIFO_ERROR         0x80

// Line control register bits
#define UART_LCR_WORD_LENGTH_5      0x00
#define UART_LCR_WORD_LENGTH_6      0x01
#define UART_LCR_WORD_LENGTH_7      0x02
#define UART_LCR_WORD_LENGTH_8      0x03
#define UART_LCR_STOP_BITS_1        0x00
#define UART_LCR_STOP_BITS_2        0x04
#define UART_LCR_PARITY_NONE        0x00
#define UART_LCR_PARITY_ODD         0x08
#define UART_LCR_PARITY_EVEN        0x18
#define UART_LCR_PARITY_MARK        0x28
#define UART_LCR_PARITY_SPACE       0x38
#define UART_LCR_BREAK_CONTROL      0x40
#define UART_LCR_DIVISOR_LATCH      0x80

// FIFO control register bits
#define UART_FCR_ENABLE_FIFO        0x01
#define UART_FCR_CLEAR_RECV_FIFO    0x02
#define UART_FCR_CLEAR_SEND_FIFO    0x04
#define UART_FCR_DMA_MODE           0x08
#define UART_FCR_TRIGGER_1          0x00
#define UART_FCR_TRIGGER_4          0x40
#define UART_FCR_TRIGGER_8          0x80
#define UART_FCR_TRIGGER_14         0xC0

// Modem control register bits
#define UART_MCR_DTR                0x01
#define UART_MCR_RTS                0x02
#define UART_MCR_OUT1               0x04
#define UART_MCR_OUT2               0x08
#define UART_MCR_LOOPBACK           0x10

// Baud rate divisors (for 115200 baud base)
#define BAUD_115200     1
#define BAUD_57600      2
#define BAUD_38400      3
#define BAUD_19200      6
#define BAUD_9600       12
#define BAUD_4800       24
#define BAUD_2400       48
#define BAUD_1200       96

// Function declarations
void serial_init(uint16_t port, uint16_t baud_divisor);
void serial_putchar(uint16_t port, char c);
void serial_puts(uint16_t port, const char* str);
char serial_getchar(uint16_t port);
int serial_available(uint16_t port);
void serial_printf(uint16_t port, const char* format, ...);

// Convenience functions for COM1
void serial_debug_init(void);
void serial_debug_putchar(char c);
void serial_debug_puts(const char* str);
void serial_debug_printf(const char* format, ...);

#endif // SERIAL_H