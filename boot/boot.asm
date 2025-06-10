; ClaudeOS Bootloader
; Simple bootloader that prints "ClaudeOS" and enters an infinite loop

[BITS 16]                   ; 16-bit real mode
[ORG 0x7C00]               ; BIOS loads bootloader at 0x7C00

start:
    ; Clear direction flag and set up segments
    cld
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00          ; Set stack pointer

    ; Print welcome message
    mov si, welcome_msg
    call print_string

    ; Infinite loop (halt system)
halt:
    cli                     ; Disable interrupts
    hlt                     ; Halt processor
    jmp halt               ; Jump back if processor wakes up

; Function to print null-terminated string
; Input: SI = pointer to string
print_string:
    mov ah, 0x0E           ; BIOS teletype function
    mov bh, 0              ; Page number
    mov bl, 0x07           ; Attribute (light gray on black)

.loop:
    lodsb                  ; Load byte from DS:SI into AL, increment SI
    cmp al, 0              ; Check for null terminator
    je .done
    int 0x10               ; BIOS video interrupt
    jmp .loop

.done:
    ret

; Data section
welcome_msg db 'ClaudeOS Bootloader v0.1', 0x0D, 0x0A
           db 'Welcome to the 70-day OS Challenge!', 0x0D, 0x0A
           db 'Day 2: Basic Bootloader', 0x0D, 0x0A, 0

; Boot signature
times 510-($-$$) db 0      ; Pad with zeros to 510 bytes
dw 0xAA55                  ; Boot sector signature