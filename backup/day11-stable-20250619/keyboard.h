#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "types.h"

// Keyboard port constants
#define KEYBOARD_DATA_PORT    0x60
#define KEYBOARD_STATUS_PORT  0x64
#define KEYBOARD_COMMAND_PORT 0x64

// Keyboard status flags
#define KEYBOARD_STATUS_OUTPUT_FULL  0x01
#define KEYBOARD_STATUS_INPUT_FULL   0x02
#define KEYBOARD_STATUS_SYSTEM       0x04
#define KEYBOARD_STATUS_COMMAND      0x08
#define KEYBOARD_STATUS_LOCKED       0x10
#define KEYBOARD_STATUS_AUX_FULL     0x20
#define KEYBOARD_STATUS_TIMEOUT      0x40
#define KEYBOARD_STATUS_PARITY       0x80

// Special scancodes
#define SCANCODE_ESC        0x01
#define SCANCODE_BACKSPACE  0x0E
#define SCANCODE_TAB        0x0F
#define SCANCODE_ENTER      0x1C
#define SCANCODE_CTRL       0x1D
#define SCANCODE_LSHIFT     0x2A
#define SCANCODE_RSHIFT     0x36
#define SCANCODE_ALT        0x38
#define SCANCODE_SPACE      0x39
#define SCANCODE_CAPS       0x3A

// Release flag (bit 7 set means key release)
#define SCANCODE_RELEASE_FLAG 0x80

// Function declarations
void keyboard_init(void);
void keyboard_handler(void);
char keyboard_get_char(void);
int keyboard_has_input(void);

#endif // KEYBOARD_H