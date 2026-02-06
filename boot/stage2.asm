; ============================================================================
; OpenWare OS - Stage 2 Bootloader
; Copyright (c) 2026 Ventryx Inc. All rights reserved.
; ============================================================================
; Second stage bootloader that:
; 1. Enables A20 line using Fast A20
; 2. Loads the Kernel (Chunked load for large kernels)
; 3. Sets up GDT
; 4. Switches to 32-bit protected mode
; 5. Jumps to kernel
; ============================================================================

[BITS 16]
[ORG 0x7E00]

KERNEL_OFFSET   equ 0x100000    ; Load kernel at 1MB
KERNEL_SECTORS  equ 800         ; Number of sectors to read (~400KB)
KERNEL_LOAD_SEG equ 0x1000      ; Temporary load segment (starts at 64KB)
KERNEL_LOAD_OFF equ 0x0000      ; Offset within segment

stage2_start:
    ; Save boot drive passed from stage1
    mov [boot_drive], dl

    ; Set up stack
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7E00
    sti

    ; Print stage2 message
    mov si, msg_stage2
    call print_string_16

    ; =========================================================================
    ; VESA VBE Initialization
    ; =========================================================================
    mov si, msg_vbe_init
    call print_string_16

    ; Get VBE Controller Info
    mov ax, 0x4F00
    mov di, 0x4000              ; Struct goes to 0x4000
    int 0x10
    cmp ax, 0x004F
    jne .vbe_fail

    ; We want 1024x768x32 or fallback to 800x600x32
    ; For simplicity in this script, we'll try to set 0x118 (1024x768x24/32) or 0x115
    ; Real-world OS would scan the mode list, but we'll try common modes directly first
    
    mov ax, 0x4F01              ; Get Mode Info
    mov cx, 0x4118              ; Mode 0x118 (1024x768x32) + LFB bit (0x4000)
    mov di, 0x5000              ; Store ModeInfoBlock at 0x5000 for kernel
    int 0x10
    cmp ax, 0x004F
    je .vbe_set

    mov ax, 0x4F01
    mov cx, 0x4115              ; Fallback: 800x600x32
    mov di, 0x5000
    int 0x10
    cmp ax, 0x004F
    jne .vbe_fail

.vbe_set:
    mov ax, 0x4F02              ; Set VBE Mode
    mov bx, cx                  ; Use the successful CX from above
    int 0x10
    cmp ax, 0x004F
    jne .vbe_fail
    
    jmp .vbe_done

.vbe_fail:
    mov si, msg_vbe_err
    call print_string_16
    ; Stay in text mode but continue (kernel will detect no VBE)
    ; Or halt? Let's halt for now since this project is ABOUT graphics
    jmp $

.vbe_done:
    ; Enable A20 line
    call enable_a20_fast

    mov si, msg_a20
    call print_string_16

    ; Load kernel message
    mov si, msg_load_kernel
    call print_string_16

    ; =========================================================================
    ; Load Kernel in Chunks (Avoid 64KB Segment Limit)
    ; =========================================================================
    mov ax, KERNEL_LOAD_SEG     ; Start buffer segment
    mov es, ax
    mov bx, KERNEL_LOAD_OFF
    
    mov ax, KERNEL_SECTORS      ; Total sectors to read
    
.read_loop:
    cmp ax, 0
    je .read_done
    
    push ax                     ; Save total remaining
    
    ; Read max 32 sectors (16KB) at a time to be safe
    cmp ax, 32
    jge .limit_32
    mov word [current_chunk], ax
    jmp .do_read
.limit_32:
    mov word [current_chunk], 32

.do_read:
    mov cx, [current_chunk]
    call read_sectors_safe      ; Reads CX sectors to ES:BX, increments lba_pos
    
    ; Advance Segment by chunk size
    ; bytes = chunk * 512
    ; paragraphs = bytes / 16 = chunk * 32
    mov dx, [current_chunk]
    shl dx, 5                   ; * 32 paragraphs
    
    mov bx, es
    add bx, dx
    mov es, bx
    
    xor bx, bx                  ; Reset offset to 0 for next segment base
    
    pop ax                      ; Restore total
    sub ax, [current_chunk]     ; Dec remaining
    jmp .read_loop

.read_done:
    ; Set up GDT
    mov si, msg_gdt
    call print_string_16
    
    lgdt [gdt_descriptor]

    ; Switch to protected mode
    mov si, msg_pmode
    call print_string_16

    cli                         ; Disable interrupts
    mov eax, cr0
    or eax, 1                   ; Set PE (Protection Enable) bit
    mov cr0, eax

    ; Far jump to flush pipeline and load CS with GDT selector
    jmp 0x08:protected_mode_start

; ============================================================================
; enable_a20_fast - Enable A20 line using System Port 0x92 (Fast A20)
; ============================================================================
enable_a20_fast:
    in al, 0x92
    test al, 2
    jnz .done
    or al, 2
    and al, 0xFE
    out 0x92, al
.done:
    ret

; ============================================================================
; print_string_16 - Print string in 16-bit mode
; ============================================================================
print_string_16:
    pusha
.loop:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    mov bh, 0
    int 0x10
    jmp .loop
.done:
    popa
    ret

; ============================================================================
; read_sectors_safe - Read CX sectors from LBA [lba_pos] to ES:BX
; ============================================================================
read_sectors_safe:
    pusha
.loop:
    cmp cx, 0
    je .done
    
    ; Convert LBA [lba_pos] to CHS
    ; Sector = (LBA % 18) + 1
    ; Head = (LBA / 18) % 2
    ; Cylinder = (LBA / 18) / 2
    mov ax, [lba_pos]
    mov dl, 18
    div dl          ; AL = LBA / 18, AH = LBA % 18
    
    inc ah
    mov cl, ah      ; CL = Sector (1-18)
    
    xor ah, ah      ; AL = LBA / 18
    mov dl, 2
    div dl          ; AL = (LBA/18)/2 (Cyl), AH = (LBA/18)%2 (Head)
    
    mov ch, al      ; CH = Cylinder
    mov dh, ah      ; DH = Head
    
    ; Setup INT 13h
    mov AH, 0x02
    mov AL, 1       ; Read 1 sector
    mov DL, [boot_drive]
    
    push ax
    int 0x13
    pop ax
    jc .error
    
    ; Advance buffer
    add bx, 512
    ; If BX overflows, that's caller's problem (but we restrict chunk size to handle this)
    
    inc word [lba_pos]
    dec cx
    jmp .loop
    
.error:
    jmp disk_error_handler
.done:
    popa
    ret

disk_error_handler:
    mov si, msg_disk_err
    call print_string_16
    jmp $

; ============================================================================
; GDT (Global Descriptor Table)
; ============================================================================
gdt_start:
    dq 0                        ; Null descriptor
gdt_code:
    dw 0xFFFF                   ; Limit (0-15)
    dw 0x0000                   ; Base (0-15)
    db 0x00                     ; Base (16-23)
    db 10011010b                ; Access: Present, Ring 0, Code, Exec/Read
    db 11001111b                ; Flags: 4KB gran, 32-bit
    db 0x00                     ; Base (24-31)
gdt_data:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b                ; Access: Present, Ring 0, Data, Read/Write
    db 11001111b
    db 0x00
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; Size
    dd gdt_start                ; Address

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; ============================================================================
; 32-bit Protected Mode Code
; ============================================================================
[BITS 32]

protected_mode_start:
    ; Set up segment registers
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000            ; Stack
    
    ; Copy kernel from buffers to 1MB (KERNEL_OFFSET)
    ; We loaded it starting at 0x10000 (KERNEL_LOAD_SEG << 4)
    ; Total KERNEL_SECTORS * 512 bytes
    
    mov esi, 0x10000            ; Source (0x1000:0000)
    mov edi, KERNEL_OFFSET      ; Dest (0x100000)
    mov ecx, KERNEL_SECTORS * 512 / 4
    rep movsd
    
    ; Jump to kernel
    jmp KERNEL_OFFSET

; ============================================================================
; Data
; ============================================================================
msg_stage2:     db 'Stage 2 Loaded', 13, 10, 0
msg_vbe_init:   db 'Initializing VESA VBE...', 13, 10, 0
msg_vbe_err:    db 'VBE Error/Not Supported!', 13, 10, 0
msg_a20:        db 'A20 Enabled', 13, 10, 0

msg_load_kernel: db 'Loading Kernel...', 13, 10, 0
msg_gdt:        db 'GDT Set Up', 13, 10, 0
msg_pmode:      db 'Entering Protected Mode...', 13, 10, 0
msg_disk_err:   db 'Disk Read Error!', 13, 10, 0
boot_drive:     db 0
current_chunk:  dw 0
lba_pos:        dw 6            ; Start LBA (Sector 6)

; Padding
times 2048-($-$$) db 0
