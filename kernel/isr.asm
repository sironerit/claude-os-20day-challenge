; ClaudeOS Interrupt Service Routines - Day 4
; Exception handlers and IRQ handlers

; External functions
extern isr_handler
extern irq_handler

; Macro to create ISR stub without error code
%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    cli                 ; Disable interrupts
    push byte 0         ; Push dummy error code
    push byte %1        ; Push interrupt number
    jmp isr_common_stub ; Jump to common handler
%endmacro

; Macro to create ISR stub with error code
%macro ISR_ERRCODE 1
global isr%1
isr%1:
    cli                 ; Disable interrupts
    push byte %1        ; Push interrupt number
    jmp isr_common_stub ; Jump to common handler
%endmacro

; Exception handlers
ISR_NOERRCODE 0   ; Divide by zero
ISR_NOERRCODE 1   ; Debug
ISR_NOERRCODE 2   ; Non Maskable Interrupt
ISR_NOERRCODE 3   ; Breakpoint
ISR_NOERRCODE 4   ; Into Detected Overflow
ISR_NOERRCODE 5   ; Out of Bounds
ISR_NOERRCODE 6   ; Invalid Opcode
ISR_NOERRCODE 7   ; No Coprocessor
ISR_ERRCODE   8   ; Double Fault (has error code)
ISR_NOERRCODE 9   ; Coprocessor Segment Overrun
ISR_ERRCODE   10  ; Bad TSS (has error code)
ISR_ERRCODE   11  ; Segment Not Present (has error code)
ISR_ERRCODE   12  ; Stack Fault (has error code)
ISR_ERRCODE   13  ; General Protection Fault (has error code)
ISR_ERRCODE   14  ; Page Fault (has error code)

; Macro to create IRQ stub
%macro IRQ 2
global irq%1
irq%1:
    cli
    push byte 0         ; No error code for IRQs
    push byte %2        ; Push IRQ number
    jmp irq_common_stub
%endmacro

; IRQ handlers (IRQ number, interrupt number)
IRQ 0, 32   ; Timer
IRQ 1, 33   ; Keyboard
IRQ 2, 34   ; Cascade
IRQ 3, 35   ; Serial 2
IRQ 4, 36   ; Serial 1
IRQ 5, 37   ; Parallel 2
IRQ 6, 38   ; Floppy
IRQ 7, 39   ; Parallel 1
IRQ 8, 40   ; RTC
IRQ 9, 41   ; Free
IRQ 10, 42  ; Free
IRQ 11, 43  ; Free
IRQ 12, 44  ; Mouse
IRQ 13, 45  ; FPU
IRQ 14, 46  ; ATA 1
IRQ 15, 47  ; ATA 2

; Common ISR handler
isr_common_stub:
    pusha               ; Push all general purpose registers
    
    mov ax, ds          ; Save data segment
    push eax
    
    mov ax, 0x10        ; Load kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    call isr_handler    ; Call C handler
    
    pop eax             ; Restore data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    popa                ; Restore all general purpose registers
    add esp, 8          ; Clean up error code and interrupt number
    sti                 ; Re-enable interrupts
    iret                ; Return from interrupt

; Common IRQ handler
irq_common_stub:
    pusha               ; Push all general purpose registers
    
    mov ax, ds          ; Save data segment
    push eax
    
    mov ax, 0x10        ; Load kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    call irq_handler    ; Call C handler
    
    pop eax             ; Restore data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    popa                ; Restore all general purpose registers
    add esp, 8          ; Clean up error code and IRQ number
    sti                 ; Re-enable interrupts
    iret                ; Return from interrupt

; GNU stack note section (prevents executable stack warning)
section .note.GNU-stack noalloc noexec nowrite progbits