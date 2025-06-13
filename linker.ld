/* ClaudeOS Linker Script - Day 2 Restart (Warning-Free Version) */

ENTRY(_start)

SECTIONS
{
    /* Load at 1MB address */
    . = 1M;

    /* Multiboot header and code section - read and execute */
    .multiboot : {
        *(.multiboot)
    } :text

    .text ALIGN(4K) : {
        *(.text)
    } :text

    /* Read-only data */
    .rodata ALIGN(4K) : {
        *(.rodata)
    } :rodata

    /* Data and BSS sections - read and write */
    .data ALIGN(4K) : {
        *(.data)
    } :data

    .bss ALIGN(4K) : {
        *(.bss)
        *(COMMON)
    } :data

    /* Discard note sections to avoid warnings */
    /DISCARD/ : {
        *(.note.GNU-stack)
        *(.comment)
    }
}

/* Program headers to control memory permissions */
PHDRS
{
    text PT_LOAD FLAGS(5);          /* Read + Execute (multiboot + code) */
    rodata PT_LOAD FLAGS(4);        /* Read only (rodata) */
    data PT_LOAD FLAGS(6);          /* Read + Write (data + bss) */
}