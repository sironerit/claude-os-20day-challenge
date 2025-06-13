#ifndef TIMER_H
#define TIMER_H

#include "../kernel/types.h"

// PIT (Programmable Interval Timer) Driver
// 8253/8254 Timer chip for generating periodic interrupts

// PIT I/O ports
#define PIT_CHANNEL0    0x40    // Channel 0 data port (IRQ0)
#define PIT_CHANNEL1    0x41    // Channel 1 data port (RAM refresh)
#define PIT_CHANNEL2    0x42    // Channel 2 data port (PC speaker)
#define PIT_COMMAND     0x43    // Mode/Command register

// PIT frequency (1.193182 MHz)
#define PIT_FREQUENCY   1193182

// Timer settings
#define TIMER_DEFAULT_FREQUENCY 100  // 100 Hz (10ms intervals)

// Command byte bits
#define PIT_SELECT_CHANNEL0     0x00
#define PIT_SELECT_CHANNEL1     0x40
#define PIT_SELECT_CHANNEL2     0x80
#define PIT_ACCESS_LATCH        0x00
#define PIT_ACCESS_LOBYTE       0x10
#define PIT_ACCESS_HIBYTE       0x20
#define PIT_ACCESS_LOHI         0x30
#define PIT_MODE_TERMINAL       0x00
#define PIT_MODE_ONESHOT        0x02
#define PIT_MODE_RATE           0x04
#define PIT_MODE_SQUARE         0x06
#define PIT_MODE_SOFTWARE       0x08
#define PIT_MODE_HARDWARE       0x0A
#define PIT_BINARY              0x00
#define PIT_BCD                 0x01

// Global timer variables
extern volatile uint32_t timer_ticks;
extern volatile uint32_t timer_seconds;

// Function declarations
void timer_init(uint32_t frequency);
void timer_handler(void);
uint32_t timer_get_ticks(void);
uint32_t timer_get_seconds(void);
void timer_wait(uint32_t ticks);

#endif // TIMER_H