; ClaudeOS IDT Flush Assembly - Day 3
; Loads the IDT

global idt_flush

idt_flush:
    mov eax, [esp+4]    ; Get IDT pointer from parameter
    lidt [eax]          ; Load IDT
    ret

; GNU stack note section (prevents executable stack warning)
section .note.GNU-stack noalloc noexec nowrite progbits