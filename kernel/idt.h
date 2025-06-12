#ifndef IDT_H
#define IDT_H

// Type definitions (matching kernel.h)
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

// IDT entry structure
struct idt_entry {
    uint16_t base_lo;        // Lower 16 bits of address to jump to when interrupt fires
    uint16_t sel;            // Kernel segment selector
    uint8_t  always0;        // Always set to zero
    uint8_t  flags;          // Flags and attributes
    uint16_t base_hi;        // Upper 16 bits of address to jump to
} __attribute__((packed));

// IDT pointer structure
struct idt_ptr {
    uint16_t limit;          // Size of the IDT-1
    uint32_t base;           // Address of the first IDT entry
} __attribute__((packed));

// IDT flags
#define IDT_FLAG_PRESENT     0x80   // Present bit
#define IDT_FLAG_RING0       0x00   // Ring 0 (kernel)
#define IDT_FLAG_RING3       0x60   // Ring 3 (user)
#define IDT_FLAG_INT32       0x0E   // 32-bit interrupt gate

// Standard interrupt/exception numbers
#define INT_DIVIDE_ERROR     0      // Division by zero
#define INT_DEBUG            1      // Debug exception
#define INT_NMI              2      // Non-maskable interrupt
#define INT_BREAKPOINT       3      // Breakpoint
#define INT_OVERFLOW         4      // Overflow
#define INT_BOUNDS           5      // Bound range exceeded
#define INT_INVALID_OPCODE   6      // Invalid opcode
#define INT_DEVICE_NOT_AVAIL 7      // Device not available
#define INT_DOUBLE_FAULT     8      // Double fault
#define INT_COPROCESSOR      9      // Coprocessor segment overrun
#define INT_INVALID_TSS      10     // Invalid TSS
#define INT_SEGMENT_NOT_PRESENT 11  // Segment not present
#define INT_STACK_FAULT      12     // Stack fault
#define INT_GENERAL_PROTECTION 13   // General protection fault
#define INT_PAGE_FAULT       14     // Page fault
#define INT_RESERVED         15     // Reserved
#define INT_FPU_ERROR        16     // x87 FPU error
#define INT_ALIGNMENT_CHECK  17     // Alignment check
#define INT_MACHINE_CHECK    18     // Machine check
#define INT_SIMD_FP_ERROR    19     // SIMD floating-point error

// Hardware interrupts (IRQs) - start at 32
#define IRQ_TIMER            32     // Timer interrupt (IRQ0)
#define IRQ_KEYBOARD         33     // Keyboard interrupt (IRQ1)

// Function declarations
void idt_init(void);
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);
extern void idt_flush(uint32_t);  // Defined in assembly

// Exception handlers (defined in assembly)
extern void isr0(void);   // Division by zero
extern void isr1(void);   // Debug
extern void isr2(void);   // NMI
extern void isr3(void);   // Breakpoint
extern void isr4(void);   // Overflow
extern void isr5(void);   // Bounds
extern void isr6(void);   // Invalid opcode
extern void isr7(void);   // Device not available
extern void isr8(void);   // Double fault
extern void isr9(void);   // Coprocessor
extern void isr10(void);  // Invalid TSS
extern void isr11(void);  // Segment not present
extern void isr12(void);  // Stack fault
extern void isr13(void);  // General protection fault
extern void isr14(void);  // Page fault
extern void isr15(void);  // Reserved
extern void isr16(void);  // FPU error
extern void isr17(void);  // Alignment check
extern void isr18(void);  // Machine check
extern void isr19(void);  // SIMD FP error

// IRQ handlers (defined in assembly)
extern void irq0(void);   // Timer
extern void irq1(void);   // Keyboard

#endif