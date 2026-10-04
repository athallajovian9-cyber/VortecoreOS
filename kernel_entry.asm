; ==============================================================================
; VortecoreOS - 64-bit Kernel Entry Assembly Trampoline (kernel_entry.asm)
; Sets up 64-bit calling conventions and invokes kernel_main()
; ==============================================================================

[BITS 64]
[GLOBAL _start]
[EXTERN kernel_main]

section .text
_start:
    ; Align stack to 16 bytes (System V AMD64 ABI specification)
    mov rsp, 0x90000
    and rsp, -16

    ; Call the C kernel entry point
    call kernel_main

    ; Infinite halt if kernel returns
.hang:
    cli
    hlt
    jmp .hang
