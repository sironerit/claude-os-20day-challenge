#ifndef TIMER_H
#define TIMER_H

#include "types.h"

// PIT (Programmable Interval Timer) Constants
#define PIT_CHANNEL0    0x40  // Channel 0 data port
#define PIT_CHANNEL1    0x41  // Channel 1 data port
#define PIT_CHANNEL2    0x42  // Channel 2 data port
#define PIT_COMMAND     0x43  // Command port

// PIT Command byte bits
#define PIT_SELECT_CHANNEL0     0x00  // Select channel 0
#define PIT_SELECT_CHANNEL1     0x40  // Select channel 1
#define PIT_SELECT_CHANNEL2     0x80  // Select channel 2

#define PIT_ACCESS_LATCH        0x00  // Latch count value
#define PIT_ACCESS_LOONLY       0x10  // Access low byte only
#define PIT_ACCESS_HIONLY       0x20  // Access high byte only
#define PIT_ACCESS_LOHI         0x30  // Access low byte then high byte

#define PIT_MODE_TERMINALCOUNT  0x00  // Mode 0: Terminal count
#define PIT_MODE_ONESHOT        0x02  // Mode 1: One shot
#define PIT_MODE_RATEGEN        0x04  // Mode 2: Rate generator
#define PIT_MODE_SQUAREWAVE     0x06  // Mode 3: Square wave
#define PIT_MODE_SWSTROBE       0x08  // Mode 4: Software strobe
#define PIT_MODE_HWSTROBE       0x0A  // Mode 5: Hardware strobe

#define PIT_BCD_BINARY          0x00  // Binary mode
#define PIT_BCD_BCD             0x01  // BCD mode

// Timer frequency
#define PIT_FREQUENCY           1193182  // PIT oscillator frequency (Hz)
#define TIMER_FREQUENCY         100      // Desired timer frequency (Hz)

// Function declarations
void timer_init(void);
void timer_handler(void);
uint32_t timer_get_ticks(void);
void timer_wait(uint32_t ticks);

#endif // TIMER_H