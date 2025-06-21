; ClaudeOS System Call Interrupt Handler - Day 8
; Assembly handler for INT 0x80 system calls

extern syscall_handler

global syscall_interrupt_handler

syscall_interrupt_handler:
    ; Save all registers
    pusha
    push ds
    push es
    push fs
    push gs
    
    ; Save current ESP
    mov esi, esp
    
    ; Switch to kernel data segment
    mov ax, 0x10        ; Kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; System call parameters are in registers:
    ; EAX = system call number
    ; EBX = arg1
    ; ECX = arg2  
    ; EDX = arg3
    
    ; Push arguments in reverse order for C calling convention
    push edx            ; arg3
    push ecx            ; arg2
    push ebx            ; arg1
    push eax            ; syscall_num
    
    ; Call the C system call handler
    call syscall_handler
    
    ; Clean up the stack (4 arguments = 16 bytes)
    add esp, 16
    
    ; Store return value in EAX (it's already there from syscall_handler)
    ; EAX will be returned to the calling process
    
    ; Restore segment registers
    pop gs
    pop fs
    pop es
    pop ds
    
    ; Restore all general-purpose registers except EAX (return value)
    ; We need to manually restore to preserve EAX
    add esp, 4          ; Skip saved EAX
    pop ecx
    pop edx
    pop ebx
    add esp, 4          ; Skip saved ESP
    pop ebp
    pop esi
    pop edi
    
    ; Return from interrupt (preserves EAX with return value)
    iret