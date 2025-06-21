; ClaudeOS System Call Handler - Day 8
; INT 0x80 interrupt handler for system calls

[BITS 32]

; Export functions
global syscall_interrupt_handler

; Import C function
extern syscall_handler

section .text

; System call interrupt handler (INT 0x80)
; Parameters passed in registers:
;   EAX = system call number
;   EBX = argument 1
;   ECX = argument 2
;   EDX = argument 3
; Return value in EAX
syscall_interrupt_handler:
    ; Save all registers (callee-saved)
    push ebp
    mov ebp, esp
    pushad
    
    ; Save segment registers
    push ds
    push es
    push fs
    push gs
    
    ; Load kernel data segment
    mov ax, 0x10    ; Kernel data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Prepare arguments for C handler
    ; Note: Arguments are already in the right registers
    ; EAX = syscall_num, EBX = arg1, ECX = arg2, EDX = arg3
    
    ; Push arguments in reverse order (C calling convention)
    push edx        ; arg3
    push ecx        ; arg2
    push ebx        ; arg1
    push eax        ; syscall_num
    
    ; Call C system call handler
    call syscall_handler
    add esp, 16     ; Clean up stack (4 arguments * 4 bytes)
    
    ; Save return value
    mov [esp + 28], eax  ; Store return value in saved EAX position
    
    ; Restore segment registers
    pop gs
    pop fs
    pop es
    pop ds
    
    ; Restore all registers
    popad
    pop ebp
    
    ; Return from interrupt
    iret

; GNU stack note section (prevents executable stack warning)
section .note.GNU-stack noalloc noexec nowrite progbits