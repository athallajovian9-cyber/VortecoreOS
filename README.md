# VortecoreOS

A custom 32-bit x86 Operating System kernel & bootloader.

[![Discord](https://img.shields.io/badge/Discord-Join%20Vortex%20Community-5865F2?logo=discord&logoColor=white)](https://discord.gg/QtyBucygQ6)

## Features
- **Master Boot Record (MBR) Bootloader**: Exactly 512 bytes with `0xAA55` magic signature.
- **Hardware Init**: Clears segments, sets up stack pointer at `0x7C00`.
- **Fast A20 Gate**: Enables memory address line 20 via system control port `0x92`.
- **Global Descriptor Table (GDT)**: Defines flat 4GB memory model (`0x08` Code, `0x10` Data).
- **Protected Mode**: Flips CPU Control Register 0 (`CR0.PE = 1`) and jumps to 32-bit mode.
- **Direct Framebuffer**: Writes directly to `0xB8000` text video memory.
- **Built-in VM Display (`vm_display.py`)**: Runs and visualizes the bootloader execution directly on your screen without requiring QEMU or VirtualBox installed.

## How to Run

Double-click:
```
RUN_VORTECORE_VM.bat
```
Or test in actual QEMU if installed:
```bash
qemu-system-x86_64 -drive format=raw,file=boot.img
```
