[BITS 32]

; Assembly code to load the IDT
global idt_flush

idt_flush:
    mov eax, [esp+4]    ; Get the pointer to the IDT, passed as a parameter
    lidt [eax]          ; Load the IDT pointer
    ret