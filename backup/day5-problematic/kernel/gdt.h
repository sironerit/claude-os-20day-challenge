#ifndef GDT_H
#define GDT_H

// Type definitions (matching kernel.h)
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

// GDT entry structure
struct gdt_entry {
    uint16_t limit_low;      // Lower 16 bits of the limit
    uint16_t base_low;       // Lower 16 bits of the base
    uint8_t  base_middle;    // Next 8 bits of the base
    uint8_t  access;         // Access flags, determine ring access level
    uint8_t  granularity;    // High 4 bits contain flags, low 4 bits contain upper limit
    uint8_t  base_high;      // Last 8 bits of the base
} __attribute__((packed));

// GDT pointer structure
struct gdt_ptr {
    uint16_t limit;          // Upper 16 bits of all selector limits
    uint32_t base;           // Address of the first gdt_entry
} __attribute__((packed));

// Access byte flags
#define GDT_ACCESS_PRESENT     0x80   // Present bit
#define GDT_ACCESS_DPL_RING0   0x00   // Ring 0 (kernel)
#define GDT_ACCESS_DPL_RING3   0x60   // Ring 3 (user)
#define GDT_ACCESS_EXECUTABLE  0x08   // Executable bit
#define GDT_ACCESS_DC          0x04   // Direction/Conforming bit
#define GDT_ACCESS_RW          0x02   // Read/Write bit
#define GDT_ACCESS_ACCESSED    0x01   // Accessed bit

// Granularity byte flags
#define GDT_GRAN_32BIT        0x40   // 32-bit protected mode
#define GDT_GRAN_4KB          0x80   // 4KB granularity

// Standard segment selectors
#define KERNEL_CODE_SELECTOR  0x08   // Kernel code segment
#define KERNEL_DATA_SELECTOR  0x10   // Kernel data segment

// Function declarations
void gdt_init(void);
void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);
extern void gdt_flush(uint32_t);  // Defined in assembly

#endif