; ClaudeOS Context Switch - Day 7 Minimal Implementation
; Simple context switching without complex features

[BITS 32]

; Export function
global switch_context

section .text

; Minimal context switch function
; Parameters:
;   [esp+4] = old_context pointer (cpu_context_t*)
;   [esp+8] = new_context pointer (cpu_context_t*)
switch_context:
    ; Get parameters
    mov eax, [esp+4]        ; old_context
    mov edx, [esp+8]        ; new_context
    
    ; Save old context (if valid)
    test eax, eax
    jz .load_new
    
    ; Save registers to old context (simplified)
    mov [eax+0], eax        ; EAX (will fix below)
    mov [eax+4], ebx        ; EBX
    mov [eax+8], ecx        ; ECX
    mov [eax+12], edx       ; EDX (will fix below)
    mov [eax+16], esi       ; ESI
    mov [eax+20], edi       ; EDI
    mov [eax+24], esp       ; ESP
    mov [eax+28], ebp       ; EBP
    
    ; Save return address as EIP
    mov ebx, [esp]
    mov [eax+32], ebx       ; EIP
    
    ; Save EFLAGS
    pushfd
    pop ebx
    mov [eax+36], ebx       ; EFLAGS
    
    ; Fix EAX and EDX values
    mov ebx, [esp+4]        ; Get original old_context
    mov [eax+0], ebx        ; Save as EAX value
    mov ebx, [esp+8]        ; Get original new_context
    mov [eax+12], ebx       ; Save as EDX value

.load_new:
    ; Load new context
    test edx, edx
    jz .done
    
    ; Load all registers except EAX/EDX (using them for pointer)
    mov ebx, [edx+4]        ; EBX
    mov ecx, [edx+8]        ; ECX
    mov esi, [edx+16]       ; ESI
    mov edi, [edx+20]       ; EDI
    mov esp, [edx+24]       ; ESP
    mov ebp, [edx+28]       ; EBP
    
    ; Load EFLAGS
    push dword [edx+36]
    popfd
    
    ; Prepare jump to new EIP
    push dword [edx+32]     ; Push new EIP
    
    ; Load final registers
    mov eax, [edx+0]        ; EAX
    mov edx, [edx+12]       ; EDX
    
    ; Jump to new process
    ret                     ; Pop EIP and jump

.done:
    ret

; GNU stack note section
section .note.GNU-stack noalloc noexec nowrite progbits