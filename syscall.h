// =============================================================================
// VortecoreOS - 64-bit System Call (Syscall) Interface (syscall.h)
// Architecture: x86_64 Fast SYSCALL / SYSRET (MSR-driven) & INT 0x80 ABI
// Syscall Convention (matches System V AMD64 ABI):
//   RAX = Syscall Number
//   RDI = Arg 1, RSI = Arg 2, RDX = Arg 3, R10 = Arg 4, R8 = Arg 5, R9 = Arg 6
// =============================================================================

#ifndef SYSCALL_H
#define SYSCALL_H

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;
typedef long long          int64_t;

// VortecoreOS Syscall Numbers
#define SYS_EXIT      0    // Exit user application (return code in RDI)
#define SYS_PRINT     1    // Print string to console (RDI = str, RSI = len)
#define SYS_READ_KEY  2    // Read character from keyboard (returns char in RAX)
#define SYS_FS_LIST   3    // List files in RAMFS
#define SYS_FS_READ   4    // Read file from RAMFS (RDI = name, RSI = buf, RDX = max)
#define SYS_FS_WRITE  5    // Write file to RAMFS (RDI = name, RSI = buf, RDX = len)
#define SYS_FS_DELETE 6    // Delete file from RAMFS (RDI = name)
#define SYS_ALLOC_MEM 7    // Allocate memory page for user space (RDI = size)
#define SYS_GET_TIME  8    // Get system ticks / timestamp

// Model Specific Registers (MSR) for x86_64 Fast SYSCALL
#define IA32_EFER_MSR   0xC0000080  // Extended Feature Enables (Bit 0: SCE = Syscall Enable)
#define IA32_STAR_MSR   0xC0000081  // Target CS/SS for syscall and sysret
#define IA32_LSTAR_MSR  0xC0000082  // Target RIP for 64-bit SYSCALL handler
#define IA32_FMASK_MSR  0xC0000084  // RFLAGS mask during syscall (clears IF, DF, etc.)

void syscall_init(void);
int64_t syscall_dispatch(uint64_t num, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5);

// User-space helper wrapper to invoke syscall instruction from Ring 3
static inline int64_t user_syscall(uint64_t num, uint64_t a1, uint64_t a2, uint64_t a3) {
    int64_t ret;
    __asm__ volatile (
        "movq %1, %%rax\n\t"
        "movq %2, %%rdi\n\t"
        "movq %3, %%rsi\n\t"
        "movq %4, %%rdx\n\t"
        "syscall\n\t"
        "movq %%rax, %0\n\t"
        : "=r"(ret)
        : "r"(num), "r"(a1), "r"(a2), "r"(a3)
        : "rax", "rdi", "rsi", "rdx", "rcx", "r11", "memory"
    );
    return ret;
}

#endif
