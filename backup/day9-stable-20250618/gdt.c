// ClaudeOS GDT Implementation - Day 3
// Global Descriptor Table setup and management

#include "gdt.h"
#include "kernel.h"

#define GDT_ENTRIES 5

// GDT entries array
struct gdt_entry gdt_entries[GDT_ENTRIES];
struct gdt_ptr gdt_ptr;

// Initialize GDT
void gdt_init(void) {
    gdt_ptr.limit = (sizeof(struct gdt_entry) * GDT_ENTRIES) - 1;
    gdt_ptr.base = (uint32_t)&gdt_entries;

    // Null descriptor (required)
    gdt_set_gate(0, 0, 0, 0, 0);
    
    // Kernel code segment
    gdt_set_gate(1, 0, 0xFFFFFFFF, 
                 GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_SYSTEM | GDT_ACCESS_EXEC | GDT_ACCESS_RW, 
                 GDT_GRAN_4K | GDT_GRAN_32BIT | 0x0F);
    
    // Kernel data segment
    gdt_set_gate(2, 0, 0xFFFFFFFF, 
                 GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_SYSTEM | GDT_ACCESS_RW, 
                 GDT_GRAN_4K | GDT_GRAN_32BIT | 0x0F);
    
    // User code segment
    gdt_set_gate(3, 0, 0xFFFFFFFF, 
                 GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_SYSTEM | GDT_ACCESS_EXEC | GDT_ACCESS_RW, 
                 GDT_GRAN_4K | GDT_GRAN_32BIT | 0x0F);
    
    // User data segment
    gdt_set_gate(4, 0, 0xFFFFFFFF, 
                 GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_SYSTEM | GDT_ACCESS_RW, 
                 GDT_GRAN_4K | GDT_GRAN_32BIT | 0x0F);

    // Load the GDT
    gdt_flush((uint32_t)&gdt_ptr);
}

// Set a GDT gate/entry
void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    // Base address
    gdt_entries[num].base_low = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high = (base >> 24) & 0xFF;

    // Limit
    gdt_entries[num].limit_low = (limit & 0xFFFF);
    gdt_entries[num].granularity = (limit >> 16) & 0x0F;

    // Granularity and access
    gdt_entries[num].granularity |= gran & 0xF0;
    gdt_entries[num].access = access;
}