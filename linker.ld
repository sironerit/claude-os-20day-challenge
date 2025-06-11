/* ClaudeOS Linker Script
   Day 3: Kernel linking configuration with Multiboot */

ENTRY(_start)

SECTIONS
{
    /* Kernel loads at 1MB mark */
    . = 0x100000;
    
    /* Multiboot header must be early in the file */
    .multiboot ALIGN(4K) : {
        *(.multiboot)
    }
    
    /* Code section */
    .text ALIGN(4K) : {
        *(.text)
    }
    
    /* Read-only data */
    .rodata ALIGN(4K) : {
        *(.rodata)
    }
    
    /* Initialized data */
    .data ALIGN(4K) : {
        *(.data)
    }
    
    /* Uninitialized data */
    .bss ALIGN(4K) : {
        *(COMMON)
        *(.bss)
    }
    
    /* End of kernel marker */
    end = .;
}