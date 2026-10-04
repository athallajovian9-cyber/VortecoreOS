// =============================================================================
// VortecoreOS - 64-bit System Call Dispatcher & Handlers (syscall.c)
// =============================================================================

#include "syscall.h"
#include "tss.h"

// Forward declarations of kernel utilities
void kprint(const char* data);
void kprint_num(uint32_t num);
void kprint_hex(uint64_t val);
char kbd_getchar(void);

typedef struct {
    char name[32];
    uint32_t size;
    uint8_t data[1024];
    uint8_t used;
} RamFile;

RamFile* vfs_lookup(const char* name);
int vfs_create(const char* name, const char* content);
int vfs_delete(const char* name);
uint32_t kstrlen(const char* s);
void kmemcpy(void* dest, const void* src, uint32_t count);

static inline void wrmsr(uint32_t msr, uint64_t val) {
    uint32_t low = val & 0xFFFFFFFF;
    uint32_t high = val >> 32;
    __asm__ volatile ("wrmsr" : : "c"(msr), "a"(low), "d"(high));
}

static inline uint64_t rdmsr(uint32_t msr) {
    uint32_t low, high;
    __asm__ volatile ("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((uint64_t)high << 32) | low;
}

// Low-level assembly trampoline for the SYSCALL instruction
// Saves caller context, invokes syscall_dispatch(), and returns via sysretq
void syscall_entry(void);

__asm__(
    ".global syscall_entry\n"
    "syscall_entry:\n"
    "    swapgs\n"                  // Switch to kernel GS segment
    "    movq %rsp, %gs:0x10\n"     // Save User RSP
    "    movq %gs:0x08, %rsp\n"     // Load Kernel Stack Pointer
    "    pushq %rcx\n"              // Save User RIP (saved in RCX by syscall)
    "    pushq %r11\n"              // Save User RFLAGS (saved in R11 by syscall)
    "    pushq %rdi\n"
    "    pushq %rsi\n"
    "    pushq %rdx\n"
    "    pushq %r10\n"
    "    pushq %r8\n"
    "    pushq %r9\n"
    // Prepare args for C syscall_dispatch:
    // arg1(RDI) = num(RAX), arg2(RSI) = a1(RDI), arg3(RDX) = a2(RSI), arg4(RCX) = a3(RDX)...
    "    movq %rdx, %rcx\n"         // arg4 = a3
    "    movq %rsi, %rdx\n"         // arg3 = a2
    "    movq %rdi, %rsi\n"         // arg2 = a1
    "    movq %rax, %rdi\n"         // arg1 = syscall number
    "    call syscall_dispatch\n"
    // Restore registers
    "    popq %r9\n"
    "    popq %r8\n"
    "    popq %r10\n"
    "    popq %rdx\n"
    "    popq %rsi\n"
    "    popq %rdi\n"
    "    popq %r11\n"              // Restore User RFLAGS
    "    popq %rcx\n"              // Restore User RIP
    "    movq %gs:0x10, %rsp\n"    // Restore User RSP
    "    swapgs\n"
    "    sysretq\n"                // Atomically returns to Ring 3 (CS=0x1B, SS=0x23)
);

void syscall_init(void) {
    // 1. Enable SYSCALL/SYSRET instructions in EFER MSR (Bit 0: SCE)
    uint64_t efer = rdmsr(IA32_EFER_MSR);
    wrmsr(IA32_EFER_MSR, efer | 1);

    // 2. Configure STAR MSR:
    // Bits 47:32 = Kernel CS/SS base (0x08)
    // Bits 63:48 = User CS/SS base (0x10 -> user CS = 0x1B, user SS = 0x23)
    uint64_t star = ((uint64_t)0x08 << 32) | ((uint64_t)0x10 << 48);
    wrmsr(IA32_STAR_MSR, star);

    // 3. Configure LSTAR MSR with address of our assembly handler
    wrmsr(IA32_LSTAR_MSR, (uint64_t)&syscall_entry);

    // 4. Configure FMASK MSR to clear Interrupt Flag (0x200) during syscall
    wrmsr(IA32_FMASK_MSR, 0x200);
}

// C-Level Syscall Dispatcher
int64_t syscall_dispatch(uint64_t num, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5) {
    switch (num) {
        case SYS_EXIT: {
            kprint("\n[SYSCALL] Process exited with status code: ");
            kprint_num((uint32_t)a1);
            kprint("\n");
            return 0;
        }

        case SYS_PRINT: {
            // a1 = const char* str, a2 = length
            const char* str = (const char*)a1;
            kprint(str);
            return (int64_t)kstrlen(str);
        }

        case SYS_READ_KEY: {
            char ch = kbd_getchar();
            return (int64_t)ch;
        }

        case SYS_FS_READ: {
            // a1 = filename, a2 = buffer, a3 = max_len
            const char* fname = (const char*)a1;
            char* buf = (char*)a2;
            RamFile* f = vfs_lookup(fname);
            if (!f) return -1; // File not found

            uint32_t bytes = f->size;
            if (bytes > a3) bytes = a3;
            kmemcpy(buf, f->data, bytes);
            return (int64_t)bytes;
        }

        case SYS_FS_WRITE: {
            // a1 = filename, a2 = content buffer, a3 = length
            const char* fname = (const char*)a1;
            const char* content = (const char*)a2;
            return (int64_t)vfs_create(fname, content);
        }

        case SYS_FS_DELETE: {
            const char* fname = (const char*)a1;
            return (int64_t)vfs_delete(fname);
        }

        case SYS_ALLOC_MEM: {
            // Allocates memory frames for user process
            uint64_t frame = pmm_alloc_frame();
            return (int64_t)frame;
        }

        case SYS_GET_TIME: {
            // Read CPU Time-Stamp Counter (RDTSC)
            uint32_t lo, hi;
            __asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi));
            return (int64_t)(((uint64_t)hi << 32) | lo);
        }

        default:
            kprint("\n[SYSCALL ERR] Invalid syscall number: ");
            kprint_num((uint32_t)num);
            kprint("\n");
            return -1;
    }
}
