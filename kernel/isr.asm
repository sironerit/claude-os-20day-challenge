[BITS 32]

; C handler function declarations
extern isr_handler
extern irq_handler

; Common ISR stub. It saves the processor state, sets up for kernel mode segments,
; calls the C-level exception handler, and finally restores the stack frame.
isr_common_stub:
    pusha                    ; Pushes edi,esi,ebp,esp,ebx,edx,ecx,eax

    mov ax, ds               ; Lower 16-bits of eax = ds.
    push eax                 ; save the data segment descriptor

    mov ax, 0x10             ; load the kernel data segment descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call isr_handler         ; Call our C handler

    pop eax                  ; reload the original data segment descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa                     ; Pops edi,esi,ebp...
    add esp, 8               ; Cleans up the pushed error code and pushed ISR number
    sti
    iret                     ; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP

; Common IRQ stub. Same as ISR but calls different handler
irq_common_stub:
    pusha                    ; Pushes edi,esi,ebp,esp,ebx,edx,ecx,eax

    mov ax, ds               ; Lower 16-bits of eax = ds.
    push eax                 ; save the data segment descriptor

    mov ax, 0x10             ; load the kernel data segment descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call irq_handler         ; Call our C handler

    pop eax                  ; reload the original data segment descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa                     ; Pops edi,esi,ebp...
    add esp, 8               ; Cleans up the pushed error code and pushed ISR number
    sti
    iret                     ; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP

; Exception handlers (ISRs)
global isr0
global isr1
global isr2
global isr3
global isr4
global isr5
global isr6
global isr7
global isr8
global isr9
global isr10
global isr11
global isr12
global isr13
global isr14
global isr15
global isr16
global isr17
global isr18
global isr19

; IRQ handlers
global irq0
global irq1

; 0: Divide Error
isr0:
    cli
    push byte 0
    push byte 0
    jmp isr_common_stub

; 1: Debug Exception
isr1:
    cli
    push byte 0
    push byte 1
    jmp isr_common_stub

; 2: Non-maskable interrupt
isr2:
    cli
    push byte 0
    push byte 2
    jmp isr_common_stub

; 3: Breakpoint
isr3:
    cli
    push byte 0
    push byte 3
    jmp isr_common_stub

; 4: Overflow
isr4:
    cli
    push byte 0
    push byte 4
    jmp isr_common_stub

; 5: Bound range exceeded
isr5:
    cli
    push byte 0
    push byte 5
    jmp isr_common_stub

; 6: Invalid opcode
isr6:
    cli
    push byte 0
    push byte 6
    jmp isr_common_stub

; 7: Device not available
isr7:
    cli
    push byte 0
    push byte 7
    jmp isr_common_stub

; 8: Double fault (has error code)
isr8:
    cli
    push byte 8
    jmp isr_common_stub

; 9: Coprocessor segment overrun
isr9:
    cli
    push byte 0
    push byte 9
    jmp isr_common_stub

; 10: Invalid TSS (has error code)
isr10:
    cli
    push byte 10
    jmp isr_common_stub

; 11: Segment not present (has error code)
isr11:
    cli
    push byte 11
    jmp isr_common_stub

; 12: Stack fault (has error code)
isr12:
    cli
    push byte 12
    jmp isr_common_stub

; 13: General protection fault (has error code)
isr13:
    cli
    push byte 13
    jmp isr_common_stub

; 14: Page fault (has error code)
isr14:
    cli
    push byte 14
    jmp isr_common_stub

; 15: Reserved
isr15:
    cli
    push byte 0
    push byte 15
    jmp isr_common_stub

; 16: x87 FPU error
isr16:
    cli
    push byte 0
    push byte 16
    jmp isr_common_stub

; 17: Alignment check
isr17:
    cli
    push byte 0
    push byte 17
    jmp isr_common_stub

; 18: Machine check
isr18:
    cli
    push byte 0
    push byte 18
    jmp isr_common_stub

; 19: SIMD floating-point error
isr19:
    cli
    push byte 0
    push byte 19
    jmp isr_common_stub

; IRQ handlers (32: Timer, 33: Keyboard)
irq0:
    cli
    push byte 0
    push byte 32
    jmp irq_common_stub

irq1:
    cli
    push byte 1
    push byte 33
    jmp irq_common_stub