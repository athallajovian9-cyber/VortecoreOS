; ==============================================================================
; VortecoreOS - 64-bit Long Mode MBR Bootloader (x86_64)
; Architecture: 16-bit Real Mode -> 32-bit Protected Mode -> 64-bit Long Mode
; Memory: 4-Level 64-bit Paging (PML4, PDPT, PD, PT) with identity mapping
; Loaded at: 0x7C00 by BIOS
; Size: Exactly 512 bytes with 0xAA55 magic boot signature
; ==============================================================================

[BITS 16]
[ORG 0x7C00]

start:
    ; 1. Clear interrupts & initialize segment registers
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    ; 2. Clear screen and set standard 80x25 VGA text mode (0x03)
    mov ah, 0x00
    mov al, 0x03
    int 0x10

    ; 3. Enable Fast A20 Gate
    in al, 0x92
    or al, 2
    out 0x92, al

    ; 4. Check if CPU supports Long Mode (CPUID.80000001H:EDX.LM[bit 29])
    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb no_long_mode

    mov eax, 0x80000001
    cpuid
    test edx, 1 << 29       ; Bit 29 is the Long Mode (LM) bit
    jz no_long_mode

    ; 5. Load 32-bit GDT and enter Protected Mode
    cli
    lgdt [gdt32_descriptor]
    mov eax, cr0
    or eax, 1               ; Set PE (Protection Enable) bit
    mov cr0, eax
    jmp CODE32_SEG:pm_entry

no_long_mode:
    mov si, err_no_lm
    call print_string
    hlt
    jmp $

print_string:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp print_string
.done:
    ret

err_no_lm db "Error: x86-64 Long Mode not supported by CPU!", 0

; ==============================================================================
; 32-bit GDT
; ==============================================================================
gdt32_start:
    dq 0x0                  ; Null descriptor
    ; Code: Base=0, Limit=0xFFFFF, Access=0x9A, Flags=0xCF
    dw 0xFFFF, 0x0000, 0x9A00, 0x00CF
    ; Data: Base=0, Limit=0xFFFFF, Access=0x92, Flags=0xCF
    dw 0xFFFF, 0x0000, 0x9200, 0x00CF
gdt32_end:

gdt32_descriptor:
    dw gdt32_end - gdt32_start - 1
    dd gdt32_start

CODE32_SEG equ 0x08
DATA32_SEG equ 0x10

; ==============================================================================
; 32-bit Protected Mode: Set up 4-Level Paging for 64-bit Long Mode
; ==============================================================================
[BITS 32]
pm_entry:
    mov ax, DATA32_SEG
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, 0x90000

    ; Set up Identity Paging at 0x1000 - 0x4000
    ; 0x1000 = PML4T, 0x2000 = PDPT, 0x3000 = PDT, 0x4000 = PT
    ; Clear 16KB of page table memory
    mov edi, 0x1000
    mov cr3, edi            ; Point CR3 to PML4 base
    xor eax, eax
    mov ecx, 4096
    rep stosd

    ; PML4[0] -> points to PDPT at 0x2000 (flags: Present | Writable = 0x03)
    mov dword [0x1000], 0x2003
    ; PDPT[0] -> points to PDT at 0x3000
    mov dword [0x2000], 0x3003
    ; PDT[0]  -> points to PT at 0x4000
    mov dword [0x3000], 0x4003

    ; Map first 2MB of memory identity-mapped (512 entries of 4KB pages in PT)
    mov edi, 0x4000
    mov ebx, 0x00000003     ; First page address: 0x0, flags: Present | Writable
    mov ecx, 512
.set_pt:
    mov [edi], ebx
    add ebx, 0x1000         ; Next 4KB frame
    add edi, 8              ; Next 64-bit entry
    loop .set_pt

    ; Enable PAE (Physical Address Extension) in CR4 (Bit 5)
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; Enable Long Mode in EFER MSR (Model Specific Register 0xC0000080, Bit 8: LME)
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; Enable Paging (CR0.PG bit 31) to activate 64-bit Long Mode
    mov eax, cr0
    or eax, (1 << 31) | 1
    mov cr0, eax

    ; Load 64-bit GDT
    lgdt [gdt64_descriptor]

    ; Far jump into 64-bit Long Mode Code Segment!
    jmp CODE64_SEG:long_mode_entry

; ==============================================================================
; 64-bit GDT (Global Descriptor Table for Long Mode)
; ==============================================================================
gdt64_start:
    dq 0x0000000000000000   ; Null descriptor
gdt64_code:
    ; 64-bit Code: Access=0x9A, Flags=0x20 (L-bit=1, D-bit=0)
    dw 0x0000, 0x0000
    db 0x00, 0x9A, 0x20, 0x00
gdt64_data:
    ; 64-bit Data: Access=0x92, Flags=0x00
    dw 0x0000, 0x0000
    db 0x00, 0x92, 0x00, 0x00
gdt64_end:

gdt64_descriptor:
    dw gdt64_end - gdt64_start - 1
    dd gdt64_start

CODE64_SEG equ gdt64_code - gdt64_start
DATA64_SEG equ gdt64_data - gdt64_start

; ==============================================================================
; 64-bit Long Mode Kernel Entry (Full 64-bit Registers RAX, RBX, RCX, RDX...)
; ==============================================================================
[BITS 64]
long_mode_entry:
    ; Reset 64-bit data segment registers
    mov ax, DATA64_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov rsp, 0x90000        ; 64-bit Stack Pointer

    ; Direct VGA Framebuffer Write (0xB8000) using 64-bit registers
    mov rsi, lm_msg
    mov rdi, 0xB8000 + (10 * 160) + (16 * 2) ; Row 10, Col 16
    mov ah, 0x1F                            ; White text on Blue background (0x1F)

.lm_loop:
    lodsb
    test al, al
    jz .halt
    mov [rdi], ax
    add rdi, 2
    jmp .lm_loop

.halt:
    hlt
    jmp .halt

lm_msg db "=== VORTECORE OS [64-BIT x86_64 LONG MODE ACTIVE] ===", 0

; Pad out to exactly 510 bytes with zeros, then append 0xAA55
times 510 - ($ - $$) db 0
dw 0xAA55
