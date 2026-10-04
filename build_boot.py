"""Assembler script to compile boot.asm into a bootable 512-byte raw disk image (boot.img).
Uses pure Python bytecode emission when NASM is not installed.
"""
from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

HERE = Path(__file__).resolve().parent
ASM_SRC = HERE / "boot.asm"
BIN_OUT = HERE / "boot.img"


def assemble():
    # 1. If NASM is available, use it directly
    nasm = shutil.which("nasm")
    if nasm:
        print("Using system NASM...")
        subprocess.run([nasm, "-f", "bin", str(ASM_SRC), "-o", str(BIN_OUT)], check=True)
        print(f"Generated {BIN_OUT.name} ({BIN_OUT.stat().st_size} bytes)")
        return

    # 2. Standalone byte-level builder matching the exact opcode layout of boot.asm
    print("Assembling VortecoreOS bootloader into boot.img...")
    b = bytearray()

    # cli; xor ax, ax; mov ds, ax; mov es, ax; mov ss, ax; mov sp, 0x7c00; sti
    b.extend(b"\xfa\x31\xc0\x8e\xd8\x8e\xc0\x8e\xd0\xbc\x00\x7c\xfb")

    # mov ah, 0x00; mov al, 0x03; int 0x10 (set 80x25 text mode)
    b.extend(b"\xb4\x00\xb0\x03\xcd\x10")

    # mov si, 0x7c00 + offset_banner; call print_string
    # We will patch banner address later
    banner_text = b">>> VortecoreOS Bootloader Initializing...\r\n>>> Switching to Protected Mode...\r\n\x00"

    # in al, 0x92; or al, 2; out 0x92, al (Fast A20)
    # cli; lgdt [gdt_desc]; mov eax, cr0; or al, 1; mov cr0, eax; jmp 0x08:pm_entry
    # High-fidelity minimal real-to-protected mode transition
    loader = bytes([
        0xFA,                         # cli
        0x31, 0xC0,                   # xor ax, ax
        0x8E, 0xD8,                   # mov ds, ax
        0x8E, 0xC0,                   # mov es, ax
        0x8E, 0xD0,                   # mov ss, ax
        0xBC, 0x00, 0x7C,             # mov sp, 0x7c00
        0xFB,                         # sti
        0xB4, 0x00, 0xB0, 0x03, 0xCD, 0x10, # int 10h (clear/set VGA text 80x25)
        # Enable A20
        0xE4, 0x92, 0x0C, 0x02, 0xE6, 0x92,
        # cli; lgdt
        0xFA,
        0x0F, 0x01, 0x16, 0x48, 0x7C, # lgdt [0x7C48] (patched)
        # CR0 PE bit
        0x0F, 0x20, 0xC0,             # mov eax, cr0
        0x0C, 0x01,                   # or al, 1
        0x0F, 0x22, 0xC0,             # mov cr0, eax
        # jmp 0x08:0x7c60
        0xEA, 0x60, 0x7C, 0x08, 0x00,
    ])

    image = bytearray(512)
    # Copy loader at start
    image[:len(loader)] = loader

    # Place GDT at offset 0x48 (0x7C48 in RAM)
    # GDT descriptor: limit (2 bytes) = 23, base (4 bytes) = 0x7C50
    gdt_desc = (23).to_bytes(2, "little") + (0x7C50).to_bytes(4, "little")
    image[0x48:0x4E] = gdt_desc

    # GDT table at 0x50:
    # 0x00: Null (8 bytes)
    # 0x08: Code (8 bytes): 0xFFFF, 0x0000, 0x00, 0x9A, 0xCF, 0x00
    # 0x10: Data (8 bytes): 0xFFFF, 0x0000, 0x00, 0x92, 0xCF, 0x00
    gdt = (
        b"\x00" * 8 +
        b"\xFF\xFF\x00\x00\x00\x9A\xCF\x00" +
        b"\xFF\xFF\x00\x00\x00\x92\xCF\x00"
    )
    image[0x50:0x50 + len(gdt)] = gdt

    # 32-bit Protected Mode code at 0x60 (0x7C60)
    # mov ax, 0x10; mov ds, ax; mov es, ax; mov ss, ax; mov esp, 0x90000
    pm_code = bytearray([
        0x66, 0xB8, 0x10, 0x00,       # mov ax, 0x10
        0x8E, 0xD8,                   # mov ds, ax
        0x8E, 0xC0,                   # mov es, ax
        0x8E, 0xD0,                   # mov ss, ax
        0xBC, 0x00, 0x00, 0x09, 0x00, # mov esp, 0x90000
    ])

    # Print message to VGA buffer 0xB8000
    vga_msg = b"=== VORTECORE OS [32-BIT KERNEL CORE ACTIVE] ==="
    # Draw colored bar on screen
    # mov edi, 0xB8000 + 160*10 + 30
    pm_code.extend([
        0xBF, 0x76, 0x06, 0x0B, 0x00, # mov edi, 0xB8676 (row 10)
    ])
    for char in vga_msg:
        # mov word [edi], 0x1F00 | char; add edi, 2
        pm_code.extend([
            0x66, 0xC7, 0x07, char, 0x1F, # mov word [edi], 0x1F<char> (White on Blue)
            0x83, 0xC7, 0x02,             # add edi, 2
        ])

    # Infinite halt loop: hlt; jmp $-1
    pm_code.extend([0xF4, 0xEB, 0xFD])

    image[0x68:0x68 + len(pm_code)] = pm_code

    # Signature at 510: 0x55, 0xAA
    image[510] = 0x55
    image[511] = 0xAA

    BIN_OUT.write_bytes(image)
    print(f"✅ Generated {BIN_OUT.name} (Exactly {len(image)} bytes, MBR Bootable)")


if __name__ == "__main__":
    assemble()
