; ============================================================================
; OpenWare OS - Stage 1 Bootloader (Boot Sector)
; Copyright (c) 2026 Ventryx Inc. All rights reserved.
; ============================================================================
; This is the first stage bootloader that fits in the 512-byte boot sector.
; It loads the second stage bootloader from disk and jumps to it.
; ============================================================================

[BITS 16]
[ORG 0x7C00]

; Constants
STAGE2_OFFSET   equ 0x7E00      ; Where to load stage2 (right after boot sector)
STAGE2_SECTORS  equ 4           ; Number of sectors to load for stage2

start:
    ; Set up segment registers
    cli                         ; Disable interrupts
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00              ; Stack grows downward from boot sector
    sti                         ; Enable interrupts

    ; Save boot drive number
    mov [boot_drive], dl

    ; Print boot message
    mov si, msg_boot
    call print_string

    ; Load Stage 2 bootloader
    mov si, msg_loading
    call print_string

    ; Reset disk system
    xor ax, ax
    mov dl, [boot_drive]
    int 0x13
    jc disk_error

    ; Load stage2 from disk
    mov ah, 0x02                ; BIOS read sectors function
    mov al, STAGE2_SECTORS      ; Number of sectors to read
    mov ch, 0                   ; Cylinder 0
    mov cl, 2                   ; Start from sector 2 (sector 1 is boot sector)
    mov dh, 0                   ; Head 0
    mov dl, [boot_drive]        ; Drive number
    mov bx, STAGE2_OFFSET       ; Buffer address ES:BX
    int 0x13
    jc disk_error

    ; Jump to stage2
    mov si, msg_jump
    call print_string
    mov dl, [boot_drive]        ; Pass boot drive to stage2 in DL
    jmp STAGE2_OFFSET

disk_error:
    mov si, msg_disk_err
    call print_string
    jmp halt

halt:
    cli
    hlt
    jmp halt

; ============================================================================
; print_string - Print null-terminated string
; Input: SI = pointer to string
; ============================================================================
print_string:
    pusha
.loop:
    lodsb                       ; Load byte from SI into AL
    or al, al                   ; Check for null terminator
    jz .done
    mov ah, 0x0E                ; BIOS teletype function
    mov bh, 0
    int 0x10
    jmp .loop
.done:
    popa
    ret

; ============================================================================
; Data
; ============================================================================
boot_drive:     db 0
msg_boot:       db 'OpenWare Boot v1.0', 13, 10, 0
msg_loading:    db 'Loading Stage 2...', 13, 10, 0
msg_jump:       db 'Jumping to Stage 2...', 13, 10, 0
msg_disk_err:   db 'Disk Error!', 13, 10, 0

; ============================================================================
; Boot sector padding and signature
; ============================================================================
times 510-($-$$) db 0           ; Pad to 510 bytes
dw 0xAA55                       ; Boot signature
