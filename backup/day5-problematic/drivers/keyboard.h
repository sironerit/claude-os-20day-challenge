#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "../kernel/types.h"

// PS/2 Keyboard Driver
// Handles keyboard input via IRQ1

// Keyboard I/O ports
#define KEYBOARD_DATA_PORT      0x60
#define KEYBOARD_STATUS_PORT    0x64
#define KEYBOARD_COMMAND_PORT   0x64

// Keyboard status register bits
#define KEYBOARD_STATUS_OUTPUT_BUFFER   0x01
#define KEYBOARD_STATUS_INPUT_BUFFER    0x02

// Special keys
#define KEY_ESC         0x01
#define KEY_BACKSPACE   0x0E
#define KEY_TAB         0x0F
#define KEY_ENTER       0x1C
#define KEY_CTRL        0x1D
#define KEY_LSHIFT      0x2A
#define KEY_RSHIFT      0x36
#define KEY_ALT         0x38
#define KEY_SPACE       0x39
#define KEY_CAPS        0x3A
#define KEY_F1          0x3B
#define KEY_F2          0x3C
#define KEY_F3          0x3D
#define KEY_F4          0x3E
#define KEY_F5          0x3F
#define KEY_F6          0x40
#define KEY_F7          0x41
#define KEY_F8          0x42
#define KEY_F9          0x43
#define KEY_F10         0x44

// Key state flags
#define KEY_FLAG_PRESSED    0x80

// Keyboard buffer size
#define KEYBOARD_BUFFER_SIZE 256

// Global keyboard variables
extern volatile char keyboard_buffer[KEYBOARD_BUFFER_SIZE];
extern volatile uint8_t keyboard_buffer_head;
extern volatile uint8_t keyboard_buffer_tail;
extern volatile uint8_t shift_pressed;
extern volatile uint8_t ctrl_pressed;
extern volatile uint8_t alt_pressed;

// Function declarations
void keyboard_init(void);
void keyboard_handler(void);
char keyboard_getchar(void);
int keyboard_available(void);
void keyboard_clear_buffer(void);
char scancode_to_ascii(uint8_t scancode, uint8_t shift);

#endif // KEYBOARD_H