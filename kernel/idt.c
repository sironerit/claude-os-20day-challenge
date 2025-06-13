// ClaudeOS IDT Implementation - Day 3
// Interrupt Descriptor Table setup and management

#include "idt.h"
#include "kernel.h"

#define IDT_ENTRIES 256

// IDT entries array
struct idt_entry idt_entries[IDT_ENTRIES];
struct idt_ptr idt_ptr;

// Initialize IDT
void idt_init(void) {
    idt_ptr.limit = sizeof(struct idt_entry) * IDT_ENTRIES - 1;
    idt_ptr.base = (uint32_t)&idt_entries;

    // Clear all IDT entries
    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt_entries[i].base_low = 0;
        idt_entries[i].selector = 0;
        idt_entries[i].always0 = 0;
        idt_entries[i].flags = 0;
        idt_entries[i].base_high = 0;
    }

    // Set up exception handlers (ISRs)
    idt_set_gate(0, (uint32_t)isr0, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT_GATE);
    idt_set_gate(1, (uint32_t)isr1, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT_GATE);
    idt_set_gate(2, (uint32_t)isr2, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT_GATE);
    idt_set_gate(3, (uint32_t)isr3, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT_GATE);
    idt_set_gate(4, (uint32_t)isr4, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT_GATE);
    idt_set_gate(5, (uint32_t)isr5, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT_GATE);
    idt_set_gate(6, (uint32_t)isr6, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT_GATE);
    idt_set_gate(7, (uint32_t)isr7, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT_GATE);
    idt_set_gate(8, (uint32_t)isr8, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT_GATE);
    idt_set_gate(10, (uint32_t)isr10, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT_GATE);
    idt_set_gate(11, (uint32_t)isr11, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT_GATE);
    idt_set_gate(12, (uint32_t)isr12, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT_GATE);
    idt_set_gate(13, (uint32_t)isr13, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT_GATE);
    idt_set_gate(14, (uint32_t)isr14, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT_GATE);

    // Set up IRQ handlers (INT 32-47)
    idt_set_gate(32, (uint32_t)irq0, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT_GATE);
    idt_set_gate(33, (uint32_t)irq1, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT_GATE);
    idt_set_gate(34, (uint32_t)irq2, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT_GATE);
    idt_set_gate(35, (uint32_t)irq3, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT_GATE);
    idt_set_gate(36, (uint32_t)irq4, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT_GATE);
    idt_set_gate(37, (uint32_t)irq5, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT_GATE);
    idt_set_gate(38, (uint32_t)irq6, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT_GATE);
    idt_set_gate(39, (uint32_t)irq7, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT_GATE);
    idt_set_gate(40, (uint32_t)irq8, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT_GATE);
    idt_set_gate(41, (uint32_t)irq9, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT_GATE);
    idt_set_gate(42, (uint32_t)irq10, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT_GATE);
    idt_set_gate(43, (uint32_t)irq11, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT_GATE);
    idt_set_gate(44, (uint32_t)irq12, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT_GATE);
    idt_set_gate(45, (uint32_t)irq13, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT_GATE);
    idt_set_gate(46, (uint32_t)irq14, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT_GATE);
    idt_set_gate(47, (uint32_t)irq15, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT_GATE);

    // Load the IDT
    idt_flush((uint32_t)&idt_ptr);
}

// Set an IDT gate/entry
void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags) {
    idt_entries[num].base_low = base & 0xFFFF;
    idt_entries[num].base_high = (base >> 16) & 0xFFFF;
    idt_entries[num].selector = selector;
    idt_entries[num].always0 = 0;
    idt_entries[num].flags = flags;
}