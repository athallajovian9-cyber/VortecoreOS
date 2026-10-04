// =============================================================================
// VortecoreOS - Task State Segment (TSS) & Ring 3 User-Mode Transition (tss.h)
// Architecture: x86_64 Long Mode GDT with Ring 0/Ring 3 Segments & 64-bit TSS
// =============================================================================

#ifndef TSS_H
#define TSS_H

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

// x86_64 Task State Segment structure (104 bytes, 16-byte aligned)
typedef struct __attribute__((packed)) {
    uint32_t reserved0;
    uint64_t rsp0;        // Ring 0 Stack Pointer (used when transitioning from Ring 3 to Ring 0)
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist1;        // Interrupt Stack Tables (IST 1..7)
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iopb_offset; // I/O Permission Bitmap Base Address
} tss_entry_t;

// 64-bit GDT Segment Selectors
// Kernel: Ring 0 (RPL=0)
#define KERNEL_CODE_SEL 0x08
#define KERNEL_DATA_SEL 0x10

// User Mode: Ring 3 (RPL=3 -> Selector | 3)
#define USER_CODE_SEL   (0x18 | 3)
#define USER_DATA_SEL   (0x20 | 3)

// TSS 16-byte System Descriptor Selector
#define TSS_SEL         0x28

void gdt_tss_init(void);
void jump_to_user_mode(uint64_t user_entry_rip, uint64_t user_stack_rsp);

#endif
