#include "idt.h"
#include "gdt.h"

// IDT with 256 entries
struct idt_entry idt_entries[256];
struct idt_ptr idt_ptr;

// Initialize the Interrupt Descriptor Table
void idt_init(void) {
    idt_ptr.limit = (sizeof(struct idt_entry) * 256) - 1;
    idt_ptr.base = (uint32_t)&idt_entries;

    // Clear the IDT
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    // Set up exception handlers (ISRs 0-19)
    idt_set_gate(0,  (uint32_t)isr0,  KERNEL_CODE_SELECTOR, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT32);
    idt_set_gate(1,  (uint32_t)isr1,  KERNEL_CODE_SELECTOR, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT32);
    idt_set_gate(2,  (uint32_t)isr2,  KERNEL_CODE_SELECTOR, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT32);
    idt_set_gate(3,  (uint32_t)isr3,  KERNEL_CODE_SELECTOR, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT32);
    idt_set_gate(4,  (uint32_t)isr4,  KERNEL_CODE_SELECTOR, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT32);
    idt_set_gate(5,  (uint32_t)isr5,  KERNEL_CODE_SELECTOR, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT32);
    idt_set_gate(6,  (uint32_t)isr6,  KERNEL_CODE_SELECTOR, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT32);
    idt_set_gate(7,  (uint32_t)isr7,  KERNEL_CODE_SELECTOR, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT32);
    idt_set_gate(8,  (uint32_t)isr8,  KERNEL_CODE_SELECTOR, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT32);
    idt_set_gate(9,  (uint32_t)isr9,  KERNEL_CODE_SELECTOR, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT32);
    idt_set_gate(10, (uint32_t)isr10, KERNEL_CODE_SELECTOR, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT32);
    idt_set_gate(11, (uint32_t)isr11, KERNEL_CODE_SELECTOR, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT32);
    idt_set_gate(12, (uint32_t)isr12, KERNEL_CODE_SELECTOR, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT32);
    idt_set_gate(13, (uint32_t)isr13, KERNEL_CODE_SELECTOR, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT32);
    idt_set_gate(14, (uint32_t)isr14, KERNEL_CODE_SELECTOR, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT32);
    idt_set_gate(15, (uint32_t)isr15, KERNEL_CODE_SELECTOR, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT32);
    idt_set_gate(16, (uint32_t)isr16, KERNEL_CODE_SELECTOR, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT32);
    idt_set_gate(17, (uint32_t)isr17, KERNEL_CODE_SELECTOR, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT32);
    idt_set_gate(18, (uint32_t)isr18, KERNEL_CODE_SELECTOR, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT32);
    idt_set_gate(19, (uint32_t)isr19, KERNEL_CODE_SELECTOR, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT32);

    // Set up hardware interrupt handlers (IRQs)
    idt_set_gate(32, (uint32_t)irq0, KERNEL_CODE_SELECTOR, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT32);
    idt_set_gate(33, (uint32_t)irq1, KERNEL_CODE_SELECTOR, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT32);

    // Load the IDT
    idt_flush((uint32_t)&idt_ptr);
}

// Set an IDT gate entry
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt_entries[num].base_lo = base & 0xFFFF;
    idt_entries[num].base_hi = (base >> 16) & 0xFFFF;
    idt_entries[num].sel = sel;
    idt_entries[num].always0 = 0;
    idt_entries[num].flags = flags;
}