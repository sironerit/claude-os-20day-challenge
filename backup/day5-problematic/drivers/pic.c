#include "pic.h"

// I/O port access functions (from kernel/io.asm)
extern void outb(uint16_t port, uint8_t data);
extern uint8_t inb(uint16_t port);

/**
 * Initialize the 8259A PIC controllers
 * Remaps IRQ 0-7 to interrupts 32-39
 * Remaps IRQ 8-15 to interrupts 40-47
 */
void pic_init(void) {
    uint8_t mask1, mask2;
    
    // Save existing masks
    mask1 = inb(PIC1_DATA);
    mask2 = inb(PIC2_DATA);
    
    // Start initialization sequence (ICW1)
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    
    // ICW2: Define vector offsets
    outb(PIC1_DATA, 0x20);  // Master PIC vector offset (IRQ0 -> INT 32)
    outb(PIC2_DATA, 0x28);  // Slave PIC vector offset (IRQ8 -> INT 40)
    
    // ICW3: Configure cascade
    outb(PIC1_DATA, 4);     // Tell master PIC that slave is at IRQ2 (0000 0100)
    outb(PIC2_DATA, 2);     // Tell slave PIC its cascade identity (0000 0010)
    
    // ICW4: Set mode
    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);
    
    // Restore saved masks
    outb(PIC1_DATA, mask1);
    outb(PIC2_DATA, mask2);
}

/**
 * Send End-of-Interrupt (EOI) signal to PIC
 * Must be called at the end of interrupt handlers
 */
void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) {
        // Send EOI to slave PIC for IRQ 8-15
        outb(PIC2_COMMAND, PIC_EOI);
    }
    // Always send EOI to master PIC
    outb(PIC1_COMMAND, PIC_EOI);
}

/**
 * Disable all IRQs by masking them
 */
void pic_disable(void) {
    outb(PIC1_DATA, 0xFF);  // Mask all IRQs on master PIC
    outb(PIC2_DATA, 0xFF);  // Mask all IRQs on slave PIC
}

/**
 * Enable a specific IRQ
 */
void pic_enable_irq(uint8_t irq) {
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

/**
 * Disable a specific IRQ
 */
void pic_disable_irq(uint8_t irq) {
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