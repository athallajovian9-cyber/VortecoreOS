# VortecoreOS

A custom 64-bit x86_64 Operating System kernel & bootloader.

[![Discord](https://img.shields.io/badge/Discord-Join%20Vortex%20Community-5865F2?logo=discord&logoColor=white)](https://discord.gg/QtyBucygQ6)

## Features
- **Phase 2 Step 4: ELF-64 Program Loader (`elf.c` & `elf.h`)**:
  - **ELF Header Parser**: Validates 64-bit ELF magic (`\x7FELF`), machine type (`EM_X86_64`), and little-endian ordering.
  - **Program Header Segment Mapper**: Iterates `PT_LOAD` segments, dynamically allocates memory frames, maps segment permissions (`PF_R`, `PF_W`, `PF_X`), and copies binary payloads into isolated user address spaces.
  - **New Shell Command**: `exec <elf_file>` to load and run binaries in Ring 3.
- **Phase 2 Step 5: Standard C Library (`libc.c` & `libc.h`)**:
  - **Formatted I/O**: `printf()` (with `%s`, `%d`, `%x`, `%c`), `putchar()`, `puts()`, and `getchar()`.
  - **Memory Management**: User-space heap allocator `malloc()` and `free()` over `SYS_ALLOC_MEM`.
  - **File Operations**: `open()`, `read()`, `write()`, and `remove()` mapped to kernel RAMFS system calls.
  - **Process Lifecycle**: `exit()` and `get_time()`.
  - **Sample Ring 3 Application (`hello.c`)**: Demonstrates dynamic heap allocation, file reading, and clean exit via libc wrappers.
- **Phase 2 Step 3: Fast SYSCALL / SYSRET Interface (`syscall.c` & `syscall.h`)**:
  - **MSR Hardware Dispatch**: Configured `IA32_EFER_MSR` (SCE bit), `IA32_STAR_MSR` (Segment targets), `IA32_LSTAR_MSR` (`0xC0000082` target handler pointer), and `IA32_FMASK_MSR` (RFLAGS mask).
  - **System V AMD64 Calling Convention**: `RAX` = syscall number, `RDI`, `RSI`, `RDX`, `R10`, `R8`, `R9` = arguments.
  - **Implemented System Calls**:
    - `SYS_EXIT` (0): Terminate process cleanly.
    - `SYS_PRINT` (1): Print string to console buffer.
    - `SYS_READ_KEY` (2): Fetch PS/2 keyboard character.
    - `SYS_FS_READ` (4) & `SYS_FS_WRITE` (5) & `SYS_FS_DELETE` (6): Read, create, and delete RAMFS files through kernel privileges.
    - `SYS_ALLOC_MEM` (7): Request new memory pages from PMM.
    - `SYS_GET_TIME` (8): Query CPU hardware timestamp counter (`rdtsc`).
  - **Context Switch Assembly Trampoline**: Saves caller state, manages stack swap via `swapgs`, and returns cleanly to Ring 3 via `sysretq`.
  - **New Shell Command**: `syscall` to test the user-space to kernel bridge.
- **Ring 3 User Mode & Task State Segment (TSS) (`tss.c` & `tss.h`)**:
  - **x86_64 GDT Structure**: Defined 64-bit Kernel Code (`0x08`), Kernel Data (`0x10`), User Code (`0x18 | 3` = `0x1B`), and User Data (`0x20 | 3` = `0x23`) with Descriptor Privilege Level 3 (`DPL=3`).
  - **16-byte Long Mode TSS**: Stores Ring 0 stack pointer (`rsp0 = 0x90000`) and I/O Permission Bitmap (IOPB) to block unauthorized user-space port I/O (`in`/`out`). Loaded into Task Register via `ltr`.
  - **Privilege Transition (`jump_to_user_mode`)**: Uses `iretq` to pop user stack pointer, user code segment, and RFLAGS to drop CPU privilege level from Ring 0 to Ring 3.
  - **New Shell Command**: `runuser <prog>` to drop into isolated Ring 3 user mode.
- **Phase 2: Virtual Memory Manager (VMM) & User-Space Isolation (`vmm.c`)**:
  - **4-Level Paging Architecture**: Traverses and maps `PML4` ➔ `PDPT` ➔ `PD` ➔ `PT` with 4KB page granularity.
  - **Physical Page Frame Allocator (PMM)**: 128MB bitmap-tracked physical memory manager.
  - **User-Space Memory Isolation**: `vmm_create_user_space()` generates dedicated, isolated PML4 address spaces per application.
  - **Hardware Ring Protection**: User pages mapped with `PTE_USER` (Ring 3), while kernel pages stay strictly Ring 0 supervisor. Any user crash or out-of-bounds pointer triggers a Page Fault (`#PF`) without crashing the core kernel.
  - **New Shell Commands**: `meminfo` (display paging and memory usage) and `spawn <prog>` (create isolated address space).
- **64-bit Long Mode MBR Bootloader (`boot64.asm`)**: Transitions cleanly:
  `16-bit Real Mode` ➔ `32-bit Protected Mode` ➔ `64-bit Long Mode`.
- **Interactive 64-bit Shell**:
  - Live REPL prompt (`vortecore-x64> `) driven by a PS/2 keyboard driver.
  - Built-in commands: `help`, `ls`, `cat`, `touch`, `rm`, `clear`, `sysinfo`, `reboot`.
- **In-Memory File System (RAMFS)**:
  - Supports file creation (`touch`), reading (`cat`), deleting (`rm`), and directory listings (`ls`).
  - Pre-seeded with system files: `readme.txt`, `version.sys`, and `motd`.
- **64-bit Kernel Core (`kernel.c`)**: Freestanding bare-metal microkernel:
  - Custom VGA terminal console driver with color styling, auto-scrolling, and cursor tracking.
  - Serial UART logging via COM1 (`0x3F8` at 38400 baud).
  - CPUID vendor identification (`GenuineIntel` / `AuthenticAMD`).
  - Formatted hexadecimal memory address printer (`kprint_hex`).
  - Kernel idle halt loop (`hlt`) with System V AMD64 ABI 16-byte stack alignment.
- **Kernel Trampoline (`kernel_entry.asm`)** & **Linker Script (`linker.ld`)**:
  - Positions kernel entry at physical address `0x100000` (1MB).
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
