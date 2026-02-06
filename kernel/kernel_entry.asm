; ============================================================================
; OpenWare OS - Kernel Entry Point
; Copyright (c) 2026 Ventryx Inc. All rights reserved.
; ============================================================================

[BITS 32]
[GLOBAL _start]
[EXTERN kmain]

section .text

_start:
    ; Set up stack
    mov esp, stack_top
    
    ; Call kernel main
    call kmain
    
    ; If kmain returns, halt
.halt:
    cli
    hlt
    jmp .halt

section .bss
    resb 16384                  ; 16KB stack
stack_top:
