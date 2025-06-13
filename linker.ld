/* ClaudeOS Linker Script - Day 2 Restart (Simple Version) */

ENTRY(_start)

SECTIONS
{
    /* Load at 1MB address */
    . = 1M;

    /* Multiboot header first */
    .multiboot : {
        *(.multiboot)
    }

    /* Code section */
    .text : {
        *(.text)
    }

    /* Read-only data */
    .rodata : {
        *(.rodata)
    }

    /* Data section */
    .data : {
        *(.data)
    }

    /* BSS section */
    .bss : {
        *(.bss)
    }
}