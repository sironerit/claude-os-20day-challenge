// ClaudeOS PIC Implementation - Day 4
// Programmable Interrupt Controller management

#include "pic.h"
#include "kernel.h"

// Initialize the PIC
void pic_init(void) {
    // Save current IRQ masks (for potential restoration)
    (void)inb(PIC1_DATA);  // Read and discard current mask
    (void)inb(PIC2_DATA);  // Read and discard current mask
    
    // Start initialization sequence in cascade mode
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    
    // Set vector offsets
    outb(PIC1_DATA, 0x20);  // Master PIC vector offset (IRQ 0-7 -> INT 32-39)
    io_wait();
    outb(PIC2_DATA, 0x28);  // Slave PIC vector offset (IRQ 8-15 -> INT 40-47)
    io_wait();
    
    // Configure cascade
    outb(PIC1_DATA, 4);     // Tell master PIC slave is at IRQ2
    io_wait();
    outb(PIC2_DATA, 2);     // Tell slave PIC its cascade identity
    io_wait();
    
    // Set 8086 mode
    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();
    
    // Restore masks (disable all IRQs initially)
    outb(PIC1_DATA, 0xFF);  // Mask all IRQs on master
    outb(PIC2_DATA, 0xFF);  // Mask all IRQs on slave
}

// Send End of Interrupt signal
void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) {
        // Send EOI to slave PIC
        outb(PIC2_COMMAND, PIC_EOI);
    }
    // Always send EOI to master PIC
    outb(PIC1_COMMAND, PIC_EOI);
}

// Set IRQ mask (disable IRQ)
void pic_set_mask(uint8_t irq) {
    uint16_t port;
    uint8_t value;
    
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    
    value = inb(port) | (1 << irq);
    outb(port, value);
}

// Clear IRQ mask (enable IRQ)
void pic_clear_mask(uint8_t irq) {
    uint16_t port;
    uint8_t value;
    
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    
    value = inb(port) & ~(1 << irq);
    outb(port, value);
}

// Get In-Service Register
uint16_t pic_get_isr(void) {
    outb(PIC1_COMMAND, 0x0B);
    outb(PIC2_COMMAND, 0x0B);
    return (inb(PIC2_COMMAND) << 8) | inb(PIC1_COMMAND);
}

// Get Interrupt Request Register
uint16_t pic_get_irr(void) {
    outb(PIC1_COMMAND, 0x0A);
    outb(PIC2_COMMAND, 0x0A);
    return (inb(PIC2_COMMAND) << 8) | inb(PIC1_COMMAND);
}