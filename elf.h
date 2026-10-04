// =============================================================================
// VortecoreOS - 64-bit ELF64 Binary Parser & Program Loader (elf.h)
// Architecture: Executable and Linkable Format (ELF-64) for x86_64
// =============================================================================

#ifndef ELF_H
#define ELF_H

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

// ELF Magic Numbers
#define ELF_MAGIC0  0x7F
#define ELF_MAGIC1  'E'
#define ELF_MAGIC2  'L'
#define ELF_MAGIC3  'F'

#define ELFCLASS64  2    // 64-bit architecture
#define ELFDATA2LSB 1    // Little-endian
#define ET_EXEC     2    // Executable file
#define EM_X86_64   62   // AMD x86-64 architecture

// Segment Types (Program Header p_type)
#define PT_NULL     0
#define PT_LOAD     1    // Loadable segment
#define PT_DYNAMIC  2
#define PT_INTERP   3
#define PT_NOTE     4
#define PT_SHLIB    5
#define PT_PHDR     6

// Segment Flags (p_flags)
#define PF_X        1    // Execute
#define PF_W        2    // Write
#define PF_R        4    // Read

// 64-bit ELF Header
typedef struct __attribute__((packed)) {
    uint8_t  e_ident[16];   // Magic number and other info
    uint16_t e_type;        // Object file type
    uint16_t e_machine;     // Architecture
    uint32_t e_version;     // Object file version
    uint64_t e_entry;       // Entry point virtual address (main function RIP)
    uint64_t e_phoff;       // Program header table file offset
    uint64_t e_shoff;       // Section header table file offset
    uint32_t e_flags;       // Processor-specific flags
    uint16_t e_ehsize;      // ELF header size in bytes
    uint16_t e_phentsize;   // Program header table entry size
    uint16_t e_phnum;       // Program header table entry count
    uint16_t e_shentsize;   // Section header table entry size
    uint16_t e_shnum;       // Section header table entry count
    uint16_t e_shstrndx;    // Section header string table index
} Elf64_Ehdr;

// 64-bit ELF Program Header
typedef struct __attribute__((packed)) {
    uint32_t p_type;        // Segment type
    uint32_t p_flags;       // Segment flags
    uint64_t p_offset;      // Segment file offset
    uint64_t p_vaddr;       // Segment virtual address
    uint64_t p_paddr;       // Segment physical address
    uint64_t p_filesz;      // Segment size in file
    uint64_t p_memsz;       // Segment size in memory
    uint64_t p_align;       // Segment alignment
} Elf64_Phdr;

int elf_validate(const void* buffer, uint64_t size);
uint64_t elf_load(const void* buffer, uint64_t size, uint64_t* out_pml4);

#endif
