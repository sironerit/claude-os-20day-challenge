#include "keyboard.h"
#include "pic.h"

// I/O port access functions (from kernel/io.asm)
extern void outb(uint16_t port, uint8_t data);
extern uint8_t inb(uint16_t port);

// Global keyboard variables
volatile char keyboard_buffer[KEYBOARD_BUFFER_SIZE];
volatile uint8_t keyboard_buffer_head = 0;
volatile uint8_t keyboard_buffer_tail = 0;
volatile uint8_t shift_pressed = 0;
volatile uint8_t ctrl_pressed = 0;
volatile uint8_t alt_pressed = 0;

// US QWERTY keyboard layout
static const char scancode_to_ascii_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', /* 9 */
    '9', '0', '-', '=', '\b', /* Backspace */
    '\t',                     /* Tab */
    'q', 'w', 'e', 'r',       /* 19 */
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', /* Enter key */
    0,                        /* 29   - Control */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', /* 39 */
    '\'', '`',   0,           /* Left shift */
    '\\', 'z', 'x', 'c', 'v', 'b', 'n',            /* 49 */
    'm', ',', '.', '/',   0,                        /* Right shift */
    '*',
    0,  /* Alt */
    ' ',  /* Space bar */
    0,  /* Caps lock */
    0,  /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    0,  /* Up Arrow */
    0,  /* Page Up */
    '-',
    0,  /* Left Arrow */
    0,
    0,  /* Right Arrow */
    '+',
    0,  /* 79 - End key*/
    0,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */
};

// Shifted characters for QWERTY layout
static const char scancode_to_ascii_shift_map[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', /* 9 */
    '(', ')', '_', '+', '\b', /* Backspace */
    '\t',                     /* Tab */
    'Q', 'W', 'E', 'R',       /* 19 */
    'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', /* Enter key */
    0,                        /* 29   - Control */
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', /* 39 */
    '"', '~',   0,            /* Left shift */
    '|', 'Z', 'X', 'C', 'V', 'B', 'N',            /* 49 */
    'M', '<', '>', '?',   0,                        /* Right shift */
    '*',
    0,  /* Alt */
    ' ',  /* Space bar */
    0,  /* Caps lock */
    0,  /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    0,  /* Up Arrow */
    0,  /* Page Up */
    '-',
    0,  /* Left Arrow */
    0,
    0,  /* Right Arrow */
    '+',
    0,  /* 79 - End key*/
    0,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */
};

/**
 * Initialize keyboard driver
 */
void keyboard_init(void) {
    // Clear keyboard buffer
    keyboard_clear_buffer();
    
    // Enable keyboard IRQ (IRQ1)
    pic_enable_irq(1);
}

/**
 * Keyboard interrupt handler (called from IRQ1)
 */
void keyboard_handler(void) {
    uint8_t scancode;
    char ascii_char;
    
    // Read scancode from keyboard
    scancode = inb(KEYBOARD_DATA_PORT);
    
    // Handle key release (high bit set)
    if (scancode & 0x80) {
        // Key released
        scancode &= 0x7F;  // Remove release flag
        
        // Update modifier key states
        switch (scancode) {
            case KEY_LSHIFT:
            case KEY_RSHIFT:
                shift_pressed = 0;
                break;
            case KEY_CTRL:
                ctrl_pressed = 0;
                break;
            case KEY_ALT:
                alt_pressed = 0;
                break;
        }
    } else {
        // Key pressed
        
        // Update modifier key states
        switch (scancode) {
            case KEY_LSHIFT:
            case KEY_RSHIFT:
                shift_pressed = 1;
                break;
            case KEY_CTRL:
                ctrl_pressed = 1;
                break;
            case KEY_ALT:
                alt_pressed = 1;
                break;
            default:
                // Convert scancode to ASCII
                ascii_char = scancode_to_ascii(scancode, shift_pressed);
                
                // Add to keyboard buffer if valid character
                if (ascii_char != 0) {
                    uint8_t next_head = (keyboard_buffer_head + 1) % KEYBOARD_BUFFER_SIZE;
                    if (next_head != keyboard_buffer_tail) {
                        keyboard_buffer[keyboard_buffer_head] = ascii_char;
                        keyboard_buffer_head = next_head;
                    }
                }
                break;
        }
    }
    
    // Send EOI to PIC
    pic_send_eoi(1);
}

/**
 * Get a character from keyboard buffer
 * Returns 0 if no character available
 */
char keyboard_getchar(void) {
    if (keyboard_buffer_head == keyboard_buffer_tail) {
        return 0;  // Buffer empty
    }
    
    char c = keyboard_buffer[keyboard_buffer_tail];
    keyboard_buffer_tail = (keyboard_buffer_tail + 1) % KEYBOARD_BUFFER_SIZE;
    return c;
}

/**
 * Check if keyboard input is available
 */
int keyboard_available(void) {
    return keyboard_buffer_head != keyboard_buffer_tail;
}

/**
 * Clear keyboard buffer
 */
void keyboard_clear_buffer(void) {
    keyboard_buffer_head = 0;
    keyboard_buffer_tail = 0;
}

/**
 * Convert scancode to ASCII character
 */
char scancode_to_ascii(uint8_t scancode, uint8_t shift) {
    if (scancode >= 128) {
        return 0;
    }
    
    if (shift) {
        return scancode_to_ascii_shift_map[scancode];
    } else {
        return scancode_to_ascii_map[scancode];
    }
}