// ClaudeOS Keyboard Implementation - Day 5
// PS/2 keyboard driver with basic ASCII conversion

#include "keyboard.h"
#include "pic.h"
#include "kernel.h"

// US QWERTY keyboard layout (lowercase)
static const char scancode_to_ascii[] = {
    0,    0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0,    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*',  0,   ' '
};

// US QWERTY keyboard layout (uppercase)
static const char scancode_to_ascii_shift[] = {
    0,    0,   '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0,    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0,    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*',  0,   ' '
};

// Keyboard state
static int shift_pressed = 0;
static int caps_lock = 0;
static int ctrl_pressed = 0;

// Simple circular buffer for keyboard input
#define KEYBOARD_BUFFER_SIZE 256
static char keyboard_buffer[KEYBOARD_BUFFER_SIZE];
static volatile int buffer_start = 0;
static volatile int buffer_end = 0;

// Initialize keyboard
void keyboard_init(void) {
    // Clear keyboard buffer
    buffer_start = 0;
    buffer_end = 0;
    
    // Reset keyboard state
    shift_pressed = 0;
    caps_lock = 0;
    ctrl_pressed = 0;
    
    // Enable keyboard IRQ (IRQ1)
    pic_clear_mask(IRQ1_KEYBOARD);
}

// Keyboard interrupt handler
void keyboard_handler(void) {
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    
    // Check if this is a key release (bit 7 set)
    if (scancode & SCANCODE_RELEASE_FLAG) {
        // Key release
        scancode &= ~SCANCODE_RELEASE_FLAG;  // Remove release flag
        
        // Handle modifier key releases
        if (scancode == SCANCODE_LSHIFT || scancode == SCANCODE_RSHIFT) {
            shift_pressed = 0;
        } else if (scancode == SCANCODE_CTRL) {
            ctrl_pressed = 0;
        }
        
        // Send EOI and return (we don't process key releases further)
        pic_send_eoi(IRQ1_KEYBOARD);
        return;
    }
    
    // Key press - handle modifier keys
    if (scancode == SCANCODE_LSHIFT || scancode == SCANCODE_RSHIFT) {
        shift_pressed = 1;
        pic_send_eoi(IRQ1_KEYBOARD);
        return;
    }
    
    if (scancode == SCANCODE_CTRL) {
        ctrl_pressed = 1;
        pic_send_eoi(IRQ1_KEYBOARD);
        return;
    }
    
    if (scancode == SCANCODE_CAPS) {
        caps_lock = !caps_lock;  // Toggle caps lock
        pic_send_eoi(IRQ1_KEYBOARD);
        return;
    }
    
    // Convert scancode to ASCII
    char ascii = 0;
    
    // Handle Ctrl key combinations
    if (ctrl_pressed) {
        // Handle Ctrl+P (scancode 0x19 for P)
        if (scancode == 0x19) {  // P key
            ascii = 0x10;  // Ctrl+P
        }
        // Handle Ctrl+N (scancode 0x31 for N)  
        else if (scancode == 0x31) {  // N key
            ascii = 0x0E;  // Ctrl+N
        }
        // Other Ctrl combinations can be added here
        else {
            ascii = 0;  // Ignore other Ctrl combinations
        }
    } else {
        // Normal ASCII conversion
        if (scancode < sizeof(scancode_to_ascii)) {
            if (shift_pressed) {
                ascii = scancode_to_ascii_shift[scancode];
            } else {
                ascii = scancode_to_ascii[scancode];
                
                // Apply caps lock to letters
                if (caps_lock && ascii >= 'a' && ascii <= 'z') {
                    ascii = ascii - 'a' + 'A';
                }
            }
        }
    }
    
    // Add to buffer if we got a valid character
    if (ascii != 0) {
        int next_end = (buffer_end + 1) % KEYBOARD_BUFFER_SIZE;
        if (next_end != buffer_start) {  // Buffer not full
            keyboard_buffer[buffer_end] = ascii;
            buffer_end = next_end;
        }
    }
    
    // Send EOI to PIC
    pic_send_eoi(IRQ1_KEYBOARD);
}

// Get a character from keyboard buffer
char keyboard_get_char(void) {
    if (buffer_start == buffer_end) {
        return 0;  // No input available
    }
    
    char c = keyboard_buffer[buffer_start];
    buffer_start = (buffer_start + 1) % KEYBOARD_BUFFER_SIZE;
    return c;
}

// Check if keyboard input is available
int keyboard_has_input(void) {
    return buffer_start != buffer_end;
}