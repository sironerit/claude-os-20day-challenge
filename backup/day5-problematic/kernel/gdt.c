#include "gdt.h"

// GDT with 5 entries (null, kernel code, kernel data, user code, user data)
struct gdt_entry gdt_entries[5];
struct gdt_ptr gdt_ptr;

// Initialize the Global Descriptor Table
void gdt_init(void) {
    gdt_ptr.limit = (sizeof(struct gdt_entry) * 5) - 1;
    gdt_ptr.base = (uint32_t)&gdt_entries;

    // Null descriptor (required)
    gdt_set_gate(0, 0, 0, 0, 0);

    // Kernel code segment
    // Base = 0x00000000, Limit = 0xFFFFFFFF
    // Access = Present + Ring 0 + Executable + Read/Write
    gdt_set_gate(1, 0, 0xFFFFFFFF, 
                 GDT_ACCESS_PRESENT | GDT_ACCESS_DPL_RING0 | GDT_ACCESS_EXECUTABLE | GDT_ACCESS_RW,
                 GDT_GRAN_32BIT | GDT_GRAN_4KB | 0x0F);

    // Kernel data segment  
    // Base = 0x00000000, Limit = 0xFFFFFFFF
    // Access = Present + Ring 0 + Read/Write
    gdt_set_gate(2, 0, 0xFFFFFFFF,
                 GDT_ACCESS_PRESENT | GDT_ACCESS_DPL_RING0 | GDT_ACCESS_RW,
                 GDT_GRAN_32BIT | GDT_GRAN_4KB | 0x0F);

    // User code segment (for future use)
    // Base = 0x00000000, Limit = 0xFFFFFFFF  
    // Access = Present + Ring 3 + Executable + Read/Write
    gdt_set_gate(3, 0, 0xFFFFFFFF,
                 GDT_ACCESS_PRESENT | GDT_ACCESS_DPL_RING3 | GDT_ACCESS_EXECUTABLE | GDT_ACCESS_RW,
                 GDT_GRAN_32BIT | GDT_GRAN_4KB | 0x0F);

    // User data segment (for future use)
    // Base = 0x00000000, Limit = 0xFFFFFFFF
    // Access = Present + Ring 3 + Read/Write  
    gdt_set_gate(4, 0, 0xFFFFFFFF,
                 GDT_ACCESS_PRESENT | GDT_ACCESS_DPL_RING3 | GDT_ACCESS_RW,
                 GDT_GRAN_32BIT | GDT_GRAN_4KB | 0x0F);

    // Load the GDT
    gdt_flush((uint32_t)&gdt_ptr);
}

// Set a GDT gate entry
void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
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