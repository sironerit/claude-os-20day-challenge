; ClaudeOS Enhanced Bootloader with Protected Mode
; Transitions from 16-bit real mode to 32-bit protected mode

[BITS 16]
[ORG 0x7C00]

; Boot sector entry point
start:
    ; Initialize segments and stack
    cli                     ; Disable interrupts
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Print initialization message
    mov si, msg_init
    call print_string_16

    ; Enable A20 line (required for accessing >1MB memory)
    call enable_a20

    ; Load Global Descriptor Table
    lgdt [gdt_descriptor]

    ; Switch to protected mode
    mov eax, cr0
    or eax, 1               ; Set PE (Protection Enable) bit
    mov cr0, eax

    ; Far jump to flush pipeline and enter 32-bit mode
    jmp CODE_SEG:protected_mode_start

; 16-bit functions
print_string_16:
    mov ah, 0x0E
.loop:
    lodsb
    cmp al, 0
    je .done
    int 0x10
    jmp .loop
.done:
    ret

; Enable A20 line using keyboard controller
enable_a20:
    ; Method 1: Try fast A20 gate
    in al, 0x92
    or al, 2
    out 0x92, al
    
    ; Method 2: Keyboard controller method (backup)
    call .wait_input
    mov al, 0xAD
    out 0x64, al            ; Disable keyboard
    
    call .wait_input
    mov al, 0xD0
    out 0x64, al            ; Read output port
    
    call .wait_output
    in al, 0x60
    push eax
    
    call .wait_input
    mov al, 0xD1
    out 0x64, al            ; Write output port
    
    call .wait_input
    pop eax
    or al, 2
    out 0x60, al            ; Enable A20
    
    call .wait_input
    mov al, 0xAE
    out 0x64, al            ; Enable keyboard
    
    call .wait_input
    ret

.wait_input:
    in al, 0x64
    test al, 2
    jnz .wait_input
    ret

.wait_output:
    in al, 0x64
    test al, 1
    jz .wait_output
    ret

; GDT (Global Descriptor Table)
gdt_start:

; Null descriptor (required)
gdt_null:
    dd 0x0
    dd 0x0

; Code segment descriptor
gdt_code:
    dw 0xFFFF       ; Limit (low)
    dw 0x0          ; Base (low)
    db 0x0          ; Base (middle)
    db 10011010b    ; Access byte: present, ring 0, code segment, executable, readable
    db 11001111b    ; Granularity: 4KB blocks, 32-bit
    db 0x0          ; Base (high)

; Data segment descriptor
gdt_data:
    dw 0xFFFF       ; Limit (low)
    dw 0x0          ; Base (low)
    db 0x0          ; Base (middle)
    db 10010010b    ; Access byte: present, ring 0, data segment, writable
    db 11001111b    ; Granularity: 4KB blocks, 32-bit
    db 0x0          ; Base (high)

gdt_end:

; GDT descriptor
gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; Size
    dd gdt_start                ; Address

; Segment selectors
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; 32-bit protected mode code
[BITS 32]
protected_mode_start:
    ; Set up segment registers
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Set up stack
    mov ebp, 0x90000
    mov esp, ebp

    ; Print success message (VGA text mode)
    mov esi, msg_protected
    mov edi, 0xB8000        ; VGA text buffer
    call print_string_32

    ; Infinite loop
.hang:
    hlt
    jmp .hang

; 32-bit print function (VGA text mode)
print_string_32:
    mov ah, 0x0F            ; White text on black background
.loop:
    lodsb
    cmp al, 0
    je .done
    stosw                   ; Store char and attribute
    jmp .loop
.done:
    ret

; Messages
msg_init db 'ClaudeOS: Initializing protected mode...', 0x0D, 0x0A, 0
msg_protected db 'ClaudeOS: Protected mode active! 32-bit OS ready.', 0

; Padding and boot signature
times 510-($-$$) db 0
dw 0xAA55