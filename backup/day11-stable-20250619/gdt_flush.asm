; ClaudeOS GDT Flush Assembly - Day 3
; Loads the GDT and updates segment registers

global gdt_flush

gdt_flush:
    mov eax, [esp+4]    ; Get GDT pointer from parameter
    lgdt [eax]          ; Load GDT
    
    mov ax, 0x10        ; 0x10 is the data segment selector
    mov ds, ax          ; Load all data segment selectors
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    jmp 0x08:.flush     ; 0x08 is the code segment selector
                        ; Far jump to flush instruction pipeline
.flush:
    ret

; GNU stack note section (prevents executable stack warning)
section .note.GNU-stack noalloc noexec nowrite progbits