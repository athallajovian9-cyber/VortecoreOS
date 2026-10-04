# VortecoreOS

A custom 64-bit x86_64 Operating System kernel & bootloader.

[![Discord](https://img.shields.io/badge/Discord-Join%20Vortex%20Community-5865F2?logo=discord&logoColor=white)](https://discord.gg/QtyBucygQ6)

## Features
- **64-bit Long Mode MBR Bootloader (`boot64.asm`)**: Transitions cleanly:
  `16-bit Real Mode` ➔ `32-bit Protected Mode` ➔ `64-bit Long Mode`.
- **Hardware CPUID Checks**: Verifies AMD64 / Intel 64 Long Mode support before activating.
- **4-Level Paging Engine**: Sets up identity mapping via `PML4T`, `PDPT`, `PDT`, and `PT` page tables.
- **Registers & Control Flags**:
  - Sets `CR4.PAE = 1` (Physical Address Extension).
  - Enables `EFER.LME = 1` (Long Mode Enable in MSR `0xC0000080`).
  - Enables `CR0.PG = 1` (Paging) to officially engage 64-bit execution.
- **64-bit GDT & Registers**: Full access to 64-bit registers (`RAX`, `RBX`, `RCX`, `RDX`, `RSI`, `RDI`, `RSP`, `R8`-`R15`).
- **Direct Framebuffer**: Writes directly to `0xB8000` text video memory with 64-bit pointers.
- **Zero-Dependency VM Display (`vm_display.py`)**: Runs and visualizes the 64-bit bootloader directly on your screen without requiring QEMU.

## How to Run

Double-click:
```
RUN_VORTECORE_VM.bat
```
Or with actual QEMU:
```bash
qemu-system-x86_64 -drive format=raw,file=boot.img
```
