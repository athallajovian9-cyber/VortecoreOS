; ==============================================================================
; VortecoreOS - Stage 1 Master Boot Record (MBR) Bootloader
; Architecture: x86 (16-bit Real Mode -> 32-bit Protected Mode transition)
; Loaded at: 0x7C00 by BIOS
; Size: Exactly 512 bytes with 0xAA55 magic boot signature
; ==============================================================================

[BITS 16]
[ORG 0x7C00]

start:
    ; 1. Clear interrupts & set segment registers to 0
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00          ; Stack grows downward from 0x7C00
    sti

    ; 2. Clear screen and set standard 80x25 16-color VGA text mode (0x03)
    mov ah, 0x00
    mov al, 0x03
    int 0x10

    ; 3. Print VortecoreOS banner in Teletype mode
    mov si, msg_banner
    call print_string

    ; 4. Enable A20 gate via Fast A20 (System Control Port A)
    in al, 0x92
    or al, 2
    out 0x92, al

    ; 5. Load Global Descriptor Table (GDT) for 32-bit Protected Mode
    cli
    lgdt [gdt_descriptor]

    ; 6. Switch to Protected Mode (set PE bit in CR0)
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; 7. Far jump to 32-bit Code Segment to flush CPU pipeline
    jmp CODE_SEG:protected_mode_entry

; --- 16-bit Real Mode Helper Routines ---
print_string:
    pusha
.loop:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E            ; BIOS teletype output
    mov bh, 0x00
    mov bl, 0x0B            ; Cyan text attribute
    int 0x10
    jmp .loop
.done:
    popa
    ret

; ==============================================================================
; Global Descriptor Table (Flat Memory Model: 4GB Addressing)
; ==============================================================================
gdt_start:
    ; Null Descriptor (mandatory 8 zero bytes)
    dd 0x0
    dd 0x0

gdt_code:
    ; Code Segment Descriptor: Base=0, Limit=0xFFFFF, Access=0x9A, Flags=0xCF
    dw 0xFFFF               ; Limit 0:15
    dw 0x0000               ; Base 0:15
    db 0x00                 ; Base 16:23
    db 10011010b            ; Access: Present, Ring 0, Executable, Readable
    db 11001111b            ; Flags: 4KB Granularity, 32-bit Protected Mode | Limit 16:19
    db 0x00                 ; Base 24:31

gdt_data:
    ; Data Segment Descriptor: Base=0, Limit=0xFFFFF, Access=0x92, Flags=0xCF
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b            ; Access: Present, Ring 0, Read/Write
    db 11001111b            ; Flags: 4KB Granularity, 32-bit
    db 0x00

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1   ; GDT Size (Limit)
    dd gdt_start                 ; GDT Address (Base)

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; Real Mode String Data
msg_banner db ">>> VortecoreOS Bootloader v0.1 Initializing...", 13, 10
           db ">>> Entering 32-bit Protected Mode...", 13, 10, 0

; ==============================================================================
; 32-bit Protected Mode Kernel Entry
; ==============================================================================
[BITS 32]
protected_mode_entry:
    ; Reload segment registers with 32-bit Data Selector
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000        ; Set 32-bit stack safe in conventional memory

    ; Direct VGA Framebuffer Write (0xB8000)
    ; Print "VORTECORE OS [RUNNING 32-BIT PROTECTED MODE]"
    mov esi, pm_msg
    mov edi, 0xB8000 + (10 * 160) + (18 * 2) ; Row 10, Col 18
    mov ah, 0x1F                            ; White text on Blue background (0x1F)

.pm_loop:
    lodsb
    test al, al
    jz .halt
    mov [edi], ax
    add edi, 2
    jmp .pm_loop

.halt:
    hlt
    jmp .halt

pm_msg db "=== VORTECORE OS [KERNEL CORE READY] ===", 0

; Pad out to exactly 510 bytes with zeros, then append 0xAA55
times 510 - ($ - $$) db 0
dw 0xAA55
