; ClaudeOS Paging Assembly Functions - Day 6
; Low-level paging operations

[BITS 32]

global vmm_load_page_directory
global vmm_enable_paging
global vmm_flush_tlb

; Load page directory into CR3
vmm_load_page_directory:
    mov eax, [esp+4]    ; Get page directory physical address
    mov cr3, eax        ; Load into CR3
    ret

; Enable paging by setting bit 31 in CR0
vmm_enable_paging:
    mov eax, cr0        ; Get CR0
    or eax, 0x80000000  ; Set paging bit (bit 31)
    mov cr0, eax        ; Enable paging
    ret

; Flush Translation Lookaside Buffer (TLB)
vmm_flush_tlb:
    mov eax, cr3        ; Get current page directory
    mov cr3, eax        ; Reload CR3 to flush TLB
    ret

; Add GNU stack note
section .note.GNU-stack noalloc noexec nowrite progbits