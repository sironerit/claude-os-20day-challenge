// ClaudeOS Timer Implementation - Day 4
// Programmable Interval Timer (PIT) driver

#include "timer.h"
#include "pic.h"
#include "kernel.h"

// Global timer tick counter
static volatile uint32_t timer_ticks = 0;

// Initialize the timer
void timer_init(void) {
    // Calculate divisor for desired frequency
    uint32_t divisor = PIT_FREQUENCY / TIMER_FREQUENCY;
    
    // Send command byte to PIT
    outb(PIT_COMMAND, PIT_SELECT_CHANNEL0 | PIT_ACCESS_LOHI | PIT_MODE_SQUAREWAVE | PIT_BCD_BINARY);
    
    // Send divisor (low byte first, then high byte)
    outb(PIT_CHANNEL0, divisor & 0xFF);
    outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF);
    
    // Enable timer IRQ (IRQ0)
    pic_clear_mask(IRQ0_TIMER);
}

// Timer interrupt handler
void timer_handler(void) {
    // Increment tick counter
    timer_ticks++;
    
    // Send EOI to PIC
    pic_send_eoi(IRQ0_TIMER);
}

// Get current tick count
uint32_t timer_get_ticks(void) {
    return timer_ticks;
}

// Wait for specified number of ticks
void timer_wait(uint32_t ticks) {
    uint32_t start_ticks = timer_ticks;
    while (timer_ticks < start_ticks + ticks) {
        asm volatile ("hlt");  // Halt until next interrupt
    }
}