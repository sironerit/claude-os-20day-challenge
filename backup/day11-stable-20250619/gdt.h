#ifndef GDT_H
#define GDT_H

#include "types.h"

// GDT Entry Structure
struct gdt_entry {
    uint16_t limit_low;      // Lower 16 bits of limit
    uint16_t base_low;       // Lower 16 bits of base
    uint8_t  base_middle;    // Next 8 bits of base
    uint8_t  access;         // Access flags
    uint8_t  granularity;    // Granularity and upper 4 bits of limit
    uint8_t  base_high;      // High 8 bits of base
} __attribute__((packed));

// GDT Pointer Structure
struct gdt_ptr {
    uint16_t limit;          // Upper 16 bits of all selector limits
    uint32_t base;           // Address of the first gdt_entry struct
} __attribute__((packed));

// GDT Access Byte Flags
#define GDT_ACCESS_PRESENT    0x80  // Present bit
#define GDT_ACCESS_RING0      0x00  // Ring 0 (kernel)
#define GDT_ACCESS_RING3      0x60  // Ring 3 (user)
#define GDT_ACCESS_SYSTEM     0x10  // System/Code-Data segment
#define GDT_ACCESS_EXEC       0x08  // Executable bit
#define GDT_ACCESS_DC         0x04  // Direction/Conforming bit
#define GDT_ACCESS_RW         0x02  // Read/Write bit
#define GDT_ACCESS_ACCESSED   0x01  // Accessed bit

// GDT Granularity Byte Flags
#define GDT_GRAN_4K          0x80   // 4K granularity
#define GDT_GRAN_32BIT       0x40   // 32-bit segment
#define GDT_GRAN_16BIT       0x00   // 16-bit segment

// Function declarations
void gdt_init(void);
void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);

// Assembly function to flush GDT
extern void gdt_flush(uint32_t);

#endif // GDT_H