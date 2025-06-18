#ifndef PIC_H
#define PIC_H

#include "types.h"

// PIC Constants
#define PIC1_COMMAND    0x20  // Master PIC command port
#define PIC1_DATA       0x21  // Master PIC data port
#define PIC2_COMMAND    0xA0  // Slave PIC command port
#define PIC2_DATA       0xA1  // Slave PIC data port

// PIC Commands
#define PIC_EOI         0x20  // End of Interrupt

// ICW (Initialization Command Words)
#define ICW1_ICW4       0x01  // ICW4 needed
#define ICW1_SINGLE     0x02  // Single mode (cascade mode if 0)
#define ICW1_INTERVAL4  0x04  // Call address interval 4 (8 if 0)
#define ICW1_LEVEL      0x08  // Level triggered (edge triggered if 0)
#define ICW1_INIT       0x10  // Initialization required

#define ICW4_8086       0x01  // 8086/88 mode
#define ICW4_AUTO       0x02  // Auto EOI
#define ICW4_BUF_SLAVE  0x08  // Buffered mode slave
#define ICW4_BUF_MASTER 0x0C  // Buffered mode master
#define ICW4_SFNM       0x10  // Special fully nested mode

// IRQ Numbers
#define IRQ0_TIMER      0
#define IRQ1_KEYBOARD   1
#define IRQ2_CASCADE    2
#define IRQ3_SERIAL2    3
#define IRQ4_SERIAL1    4
#define IRQ5_PARALLEL2  5
#define IRQ6_FLOPPY     6
#define IRQ7_PARALLEL1  7
#define IRQ8_RTC        8
#define IRQ9_FREE       9
#define IRQ10_FREE      10
#define IRQ11_FREE      11
#define IRQ12_MOUSE     12
#define IRQ13_FPU       13
#define IRQ14_ATA1      14
#define IRQ15_ATA2      15

// Function declarations
void pic_init(void);
void pic_send_eoi(uint8_t irq);
void pic_set_mask(uint8_t irq);
void pic_clear_mask(uint8_t irq);
uint16_t pic_get_isr(void);
uint16_t pic_get_irr(void);

// I/O port functions
void outb(uint16_t port, uint8_t data);
uint8_t inb(uint16_t port);
void io_wait(void);

#endif // PIC_H