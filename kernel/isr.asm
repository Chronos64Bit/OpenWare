; ============================================================================
; OpenWare OS - Interrupt Service Routines (ISR) Assembly Stubs
; Copyright (c) 2026 Ventryx Inc. All rights reserved.
; ============================================================================

[BITS 32]

; External C handler
extern isr_handler

; Macro for ISR with no error code
%macro ISR_NOERR 1
global isr%1
isr%1:
    push dword 0                ; Dummy error code
    push dword %1               ; Interrupt number
    jmp isr_common_stub
%endmacro

; Macro for ISR with error code
%macro ISR_ERR 1
global isr%1
isr%1:
    push dword %1               ; Interrupt number (error code already pushed)
    jmp isr_common_stub
%endmacro

; Exception ISRs
ISR_NOERR 0                     ; Division by zero
ISR_NOERR 1                     ; Debug
ISR_NOERR 2                     ; NMI
ISR_NOERR 3                     ; Breakpoint
ISR_NOERR 4                     ; Overflow
ISR_NOERR 5                     ; Bound range exceeded
ISR_NOERR 6                     ; Invalid opcode
ISR_NOERR 7                     ; Device not available
ISR_ERR   8                     ; Double fault
ISR_NOERR 9                     ; Coprocessor segment overrun
ISR_ERR   10                    ; Invalid TSS
ISR_ERR   11                    ; Segment not present
ISR_ERR   12                    ; Stack segment fault
ISR_ERR   13                    ; General protection fault
ISR_ERR   14                    ; Page fault
ISR_NOERR 15                    ; Reserved
ISR_NOERR 16                    ; x87 FPU exception
ISR_ERR   17                    ; Alignment check
ISR_NOERR 18                    ; Machine check
ISR_NOERR 19                    ; SIMD FPU exception

; Common ISR stub
isr_common_stub:
    ; Save all registers
    pusha
    
    ; Save segment registers
    push ds
    push es
    push fs
    push gs
    
    ; Load kernel data segment
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Get interrupt number and error code from stack
    mov eax, [esp + 48]         ; Interrupt number
    mov ebx, [esp + 52]         ; Error code
    
    ; Call C handler
    push ebx                    ; Error code
    push eax                    ; Interrupt number
    call isr_handler
    add esp, 8                  ; Clean up arguments
    
    ; Restore segment registers
    pop gs
    pop fs
    pop es
    pop ds
    
    ; Restore all registers
    popa
    
    ; Clean up interrupt number and error code
    add esp, 8
    
    ; Return from interrupt
    iret

; ============================================================================
; GDT and IDT flush functions
; ============================================================================
global gdt_flush
gdt_flush:
    mov eax, [esp + 4]          ; Get GDT pointer
    lgdt [eax]                  ; Load GDT
    
    ; Reload segment registers
    mov ax, 0x10                ; Data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Far jump to reload CS
    jmp 0x08:.flush
.flush:
    ret

global idt_flush
idt_flush:
    mov eax, [esp + 4]          ; Get IDT pointer
    lidt [eax]                  ; Load IDT
    ret

; IRQ Handlers
extern irq_handler

%macro IRQ 2
global irq%1
irq%1:
    push dword 0
    push dword %2
    jmp irq_common_stub
%endmacro

IRQ 0, 32
IRQ 1, 33
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39
IRQ 8, 40
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

irq_common_stub:
    pusha
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp          ; Pass pointer to registers
    call irq_handler
    add esp, 4

    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8
    iret

