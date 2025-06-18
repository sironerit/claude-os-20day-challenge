; ClaudeOS Context Switch Implementation - Day 7
; Low-level assembly code for process context switching

[BITS 32]

; Export function
global context_switch

section .text

; Context switch function
; Parameters:
;   [esp+4] = old_context pointer (cpu_context_t*)
;   [esp+8] = new_context pointer (cpu_context_t*)
context_switch:
    ; Get parameters
    mov eax, [esp+4]        ; old_context
    mov edx, [esp+8]        ; new_context
    
    ; Save old context (if not null)
    test eax, eax
    jz load_new_context
    
    ; Save general-purpose registers
    mov [eax+0], eax        ; Save old EAX (will be overwritten below)
    mov [eax+4], ebx        ; Save EBX
    mov [eax+8], ecx        ; Save ECX
    mov [eax+12], edx       ; Save EDX (will be overwritten below)
    mov [eax+16], esi       ; Save ESI
    mov [eax+20], edi       ; Save EDI
    
    ; Save stack pointers
    mov [eax+24], esp       ; Save ESP
    mov [eax+28], ebp       ; Save EBP
    
    ; Save return address as EIP
    mov ebx, [esp]          ; Get return address from stack
    mov [eax+32], ebx       ; Save as EIP
    
    ; Save flags
    pushfd                  ; Push flags onto stack
    pop ebx                 ; Pop into EBX
    mov [eax+36], ebx       ; Save EFLAGS
    
    ; Save CR3 (page directory)
    mov ebx, cr3
    mov [eax+40], ebx       ; Save CR3
    
    ; Restore actual EAX and EDX values (they were overwritten)
    push eax                ; Save old_context pointer
    push edx                ; Save new_context pointer
    
    mov edx, [esp+4]        ; Get new_context back
    mov ebx, [esp+8]        ; Get old_context back
    mov eax, [ebx+0]        ; Get original EAX value
    mov [ebx+0], eax        ; Store it back
    mov eax, [ebx+12]       ; Get original EDX value
    mov [ebx+12], eax       ; Store it back
    
    pop edx                 ; Restore new_context pointer
    pop eax                 ; We don't need old_context anymore

load_new_context:
    ; Load new context
    test edx, edx
    jz context_switch_done
    
    ; Load CR3 first (page directory)
    mov eax, [edx+40]       ; Load new CR3
    mov cr3, eax            ; Switch page directory
    
    ; Load general-purpose registers
    mov eax, [edx+0]        ; Load EAX
    mov ebx, [edx+4]        ; Load EBX
    mov ecx, [edx+8]        ; Load ECX
    ; EDX will be loaded last
    mov esi, [edx+16]       ; Load ESI
    mov edi, [edx+20]       ; Load EDI
    
    ; Load stack pointers
    mov esp, [edx+24]       ; Load ESP
    mov ebp, [edx+28]       ; Load EBP
    
    ; Load flags
    push dword [edx+36]     ; Push EFLAGS onto stack
    popfd                   ; Pop into flags register
    
    ; Prepare to jump to new EIP
    push dword [edx+32]     ; Push new EIP onto stack
    
    ; Load EDX last (we've been using it)
    mov edx, [edx+12]       ; Load EDX
    
    ; Jump to new process
    ret                     ; Pop EIP and jump to new process

context_switch_done:
    ret

; GNU stack note section (prevents executable stack warning)
section .note.GNU-stack noalloc noexec nowrite progbits