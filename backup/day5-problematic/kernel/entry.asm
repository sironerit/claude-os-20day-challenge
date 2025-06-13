; ClaudeOS Kernel Entry Point
; Day 3: 32-bit kernel entry with Multiboot support

; Multiboot header
MULTIBOOT_MAGIC         equ 0x1BADB002
MULTIBOOT_PAGE_ALIGN    equ 1 << 0
MULTIBOOT_MEMORY_INFO   equ 1 << 1
MULTIBOOT_FLAGS         equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO
MULTIBOOT_CHECKSUM      equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

global _start
extern kernel_main

section .multiboot
align 4
    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHECKSUM

section .text
bits 32

_start:
    ; Set up stack pointer
    mov esp, stack_top
    
    ; Clear direction flag for string operations
    cld
    
    ; Call main kernel function
    call kernel_main
    
    ; If kernel_main returns (shouldn't happen), halt
.hang:
    cli         ; Clear interrupts
    hlt         ; Halt processor
    jmp .hang   ; If somehow we continue, loop

section .bss
align 16
stack_bottom:
    resb 16384  ; 16 KB stack
stack_top: