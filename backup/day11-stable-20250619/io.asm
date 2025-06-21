; ClaudeOS I/O Port Operations - Day 4
; Assembly functions for hardware port access

global outb
global inb
global outw
global inw
global io_wait

; Write byte to I/O port
; void outb(uint16_t port, uint8_t data)
outb:
    mov al, [esp + 8]    ; Get data (second parameter)
    mov dx, [esp + 4]    ; Get port (first parameter)
    out dx, al           ; Output data to port
    ret

; Read byte from I/O port
; uint8_t inb(uint16_t port)
inb:
    mov dx, [esp + 4]    ; Get port (first parameter)
    in al, dx            ; Input data from port
    ret

; Write word to I/O port
; void outw(uint16_t port, uint16_t data)
outw:
    mov ax, [esp + 8]    ; Get data (second parameter)
    mov dx, [esp + 4]    ; Get port (first parameter)
    out dx, ax           ; Output data to port
    ret

; Read word from I/O port
; uint16_t inw(uint16_t port)
inw:
    mov dx, [esp + 4]    ; Get port (first parameter)
    in ax, dx            ; Input data from port
    ret

; I/O wait function (small delay for older hardware)
; void io_wait(void)
io_wait:
    out 0x80, al         ; Write to unused port for delay
    ret

; GNU stack note section (prevents executable stack warning)
section .note.GNU-stack noalloc noexec nowrite progbits