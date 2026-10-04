// =============================================================================
// VortecoreOS - GDT with Ring 3 User-Mode & 64-bit Task State Segment (tss.c)
// =============================================================================

#include "tss.h"

// GDT Entries:
// 0: Null Descriptor (8 bytes)
// 1: Kernel Code Segment (0x08, Ring 0)
// 2: Kernel Data Segment (0x10, Ring 0)
// 3: User Code Segment   (0x18, Ring 3, DPL=3)
// 4: User Data Segment   (0x20, Ring 3, DPL=3)
// 5 & 6: TSS Descriptor  (0x28, 16 bytes in x86_64 Long Mode)

static uint64_t gdt64[7];
static tss_entry_t kernel_tss __attribute__((aligned(16)));

// GDT Pointer structure (Passed to lgdt instruction)
typedef struct __attribute__((packed)) {
    uint16_t limit;
    uint64_t base;
} gdt_ptr_t;

static gdt_ptr_t gdt_ptr;

// Setup a 16-byte TSS descriptor in x86_64 Long Mode GDT
static void set_tss_descriptor(int index, uint64_t base, uint32_t limit) {
    // Low 8 bytes
    gdt64[index] = (limit & 0xFFFFULL) |
                   ((base & 0xFFFFULL) << 16) |
                   (((base >> 16) & 0xFFULL) << 32) |
                   (0x89ULL << 40) | // Type: 64-bit Available TSS (0x9), Present (0x80) -> 0x89
                   (((uint64_t)(limit >> 16) & 0x0FULL) << 48) |
                   (((base >> 24) & 0xFFULL) << 56);

    // High 8 bytes (holds upper 32 bits of base address)
    gdt64[index + 1] = (base >> 32) & 0xFFFFFFFFULL;
}

void gdt_tss_init(void) {
    // 0: Null
    gdt64[0] = 0x0000000000000000ULL;

    // 1: Kernel Code (Ring 0): Access=0x9A, Flags=0x20 (L=1)
    gdt64[1] = 0x00209A0000000000ULL;

    // 2: Kernel Data (Ring 0): Access=0x92, Flags=0x00
    gdt64[2] = 0x0000920000000000ULL;

    // 3: User Code (Ring 3, DPL=3): Access=0xFA (0xF0 | 0x0A), Flags=0x20 (L=1)
    gdt64[3] = 0x0020FA0000000000ULL;

    // 4: User Data (Ring 3, DPL=3): Access=0xF2 (0xF0 | 0x02), Flags=0x00
    gdt64[4] = 0x0000F20000000000ULL;

    // Initialize 64-bit Task State Segment (TSS)
    uint8_t* tss_bytes = (uint8_t*)&kernel_tss;
    for (int i = 0; i < sizeof(kernel_tss); i++) {
        tss_bytes[i] = 0;
    }

    // Allocate safe Ring 0 stack when interrupts occur in Ring 3
    kernel_tss.rsp0 = 0x90000;
    kernel_tss.iopb_offset = sizeof(kernel_tss); // Disable I/O port permissions for Ring 3

    // 5 & 6: TSS Descriptor (16-byte System Segment)
    set_tss_descriptor(5, (uint64_t)&kernel_tss, sizeof(kernel_tss) - 1);

    // Load GDT
    gdt_ptr.limit = sizeof(gdt64) - 1;
    gdt_ptr.base = (uint64_t)&gdt64[0];
    __asm__ volatile ("lgdt %0" : : "m"(gdt_ptr));

    // Load Task Register (LTR) with TSS selector (0x28)
    __asm__ volatile ("ltr %%ax" : : "a"((uint16_t)TSS_SEL));
}

// Drops CPU privilege down to Ring 3 using iretq (Interrupt Return 64-bit)
// iretq pops: [RIP] [CS] [RFLAGS] [RSP] [SS]
void jump_to_user_mode(uint64_t user_entry_rip, uint64_t user_stack_rsp) {
    uint64_t user_cs = USER_CODE_SEL;
    uint64_t user_ss = USER_DATA_SEL;
    uint64_t rflags  = 0x202; // IF (Interrupt Flag) enabled + Reserved Bit 1

    __asm__ volatile (
        "cli\n\t"
        "pushq %0\n\t"        // Push User SS (Ring 3 Data Selector)
        "pushq %1\n\t"        // Push User RSP (Ring 3 Stack Pointer)
        "pushq %2\n\t"        // Push RFLAGS (Interrupts Enabled)
        "pushq %3\n\t"        // Push User CS (Ring 3 Code Selector)
        "pushq %4\n\t"        // Push User RIP (Instruction Pointer)
        "iretq\n\t"           // Return to Ring 3 User Mode!
        :
        : "r"(user_ss), "r"(user_stack_rsp), "r"(rflags), "r"(user_cs), "r"(user_entry_rip)
        : "memory"
    );
}
