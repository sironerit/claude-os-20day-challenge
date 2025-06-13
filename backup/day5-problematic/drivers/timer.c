#include "timer.h"
#include "pic.h"

// I/O port access functions (from kernel/io.asm)
extern void outb(uint16_t port, uint8_t data);
extern uint8_t inb(uint16_t port);

// Global timer variables
volatile uint32_t timer_ticks = 0;
volatile uint32_t timer_seconds = 0;

// Internal variables
static uint32_t timer_frequency = TIMER_DEFAULT_FREQUENCY;
static uint32_t ticks_per_second = TIMER_DEFAULT_FREQUENCY;

/**
 * Initialize the PIT (Programmable Interval Timer)
 * Sets up Channel 0 to generate IRQ0 at specified frequency
 */
void timer_init(uint32_t frequency) {
    timer_frequency = frequency;
    ticks_per_second = frequency;
    
    // Calculate divisor for desired frequency
    uint32_t divisor = PIT_FREQUENCY / frequency;
    
    // Send command byte:
    // Channel 0, Access lo/hi bytes, Mode 3 (square wave), Binary mode
    uint8_t command = PIT_SELECT_CHANNEL0 | PIT_ACCESS_LOHI | PIT_MODE_SQUARE | PIT_BINARY;
    outb(PIT_COMMAND, command);
    
    // Send divisor (low byte first, then high byte)
    outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0, (uint8_t)((divisor >> 8) & 0xFF));
    
    // Enable timer IRQ (IRQ0)
    pic_enable_irq(0);
}

/**
 * Timer interrupt handler (called from IRQ0)
 * Updates global tick counter and seconds
 */
void timer_handler(void) {
    timer_ticks++;
    
    // Update seconds counter
    if (timer_ticks % ticks_per_second == 0) {
        timer_seconds++;
    }
    
    // Send EOI to PIC
    pic_send_eoi(0);
}

/**
 * Get current timer ticks
 */
uint32_t timer_get_ticks(void) {
    return timer_ticks;
}

/**
 * Get current seconds since boot
 */
uint32_t timer_get_seconds(void) {
    return timer_seconds;
}

/**
 * Wait for specified number of timer ticks
 */
void timer_wait(uint32_t ticks) {
    uint32_t start_ticks = timer_ticks;
    while (timer_ticks < start_ticks + ticks) {
        // Busy wait (could be improved with halt instruction)
        __asm__ volatile ("hlt");
    }
}