#ifndef IDT_H
#define IDT_H

#include "types.h"

// IDT Entry Structure
struct idt_entry {
    uint16_t base_low;       // Lower 16 bits of handler address
    uint16_t selector;       // Kernel segment selector
    uint8_t  always0;        // Always 0
    uint8_t  flags;          // Flags (type and attributes)
    uint16_t base_high;      // Upper 16 bits of handler address
} __attribute__((packed));

// IDT Pointer Structure
struct idt_ptr {
    uint16_t limit;          // Upper 16 bits of all selector limits
    uint32_t base;           // Address of the first idt_entry struct
} __attribute__((packed));

// IDT Flags
#define IDT_FLAG_PRESENT      0x80  // Present bit
#define IDT_FLAG_RING0        0x00  // Ring 0 (kernel)
#define IDT_FLAG_RING3        0x60  // Ring 3 (user)
#define IDT_FLAG_INT_GATE     0x0E  // 32-bit interrupt gate
#define IDT_FLAG_TRAP_GATE    0x0F  // 32-bit trap gate

// Exception numbers
#define EXCEPTION_DIVIDE_ERROR        0
#define EXCEPTION_DEBUG               1
#define EXCEPTION_NMI                 2
#define EXCEPTION_BREAKPOINT          3
#define EXCEPTION_OVERFLOW            4
#define EXCEPTION_BOUND_RANGE         5
#define EXCEPTION_INVALID_OPCODE      6
#define EXCEPTION_DEVICE_NOT_AVAIL    7
#define EXCEPTION_DOUBLE_FAULT        8
#define EXCEPTION_INVALID_TSS         10
#define EXCEPTION_SEGMENT_NOT_PRESENT 11
#define EXCEPTION_STACK_FAULT         12
#define EXCEPTION_GENERAL_PROTECTION  13
#define EXCEPTION_PAGE_FAULT          14

// Function declarations
void idt_init(void);
void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags);

// Assembly function to flush IDT
extern void idt_flush(uint32_t);

// Exception handlers
extern void isr0(void);   // Divide by zero
extern void isr1(void);   // Debug
extern void isr2(void);   // NMI
extern void isr3(void);   // Breakpoint
extern void isr4(void);   // Overflow
extern void isr5(void);   // Bound Range Exceeded
extern void isr6(void);   // Invalid Opcode
extern void isr7(void);   // Device Not Available
extern void isr8(void);   // Double Fault
extern void isr10(void);  // Invalid TSS
extern void isr11(void);  // Segment Not Present
extern void isr12(void);  // Stack Fault
extern void isr13(void);  // General Protection
extern void isr14(void);  // Page Fault

// IRQ handlers
extern void irq0(void);   // Timer
extern void irq1(void);   // Keyboard
extern void irq2(void);   // Cascade
extern void irq3(void);   // Serial 2
extern void irq4(void);   // Serial 1
extern void irq5(void);   // Parallel 2
extern void irq6(void);   // Floppy
extern void irq7(void);   // Parallel 1
extern void irq8(void);   // RTC
extern void irq9(void);   // Free
extern void irq10(void);  // Free
extern void irq11(void);  // Free
extern void irq12(void);  // Mouse
extern void irq13(void);  // FPU
extern void irq14(void);  // ATA 1
extern void irq15(void);  // ATA 2

// System call handler
extern void syscall_interrupt_handler(void);  // System calls (INT 0x80)

#endif // IDT_H