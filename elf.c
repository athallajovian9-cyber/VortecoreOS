// =============================================================================
// VortecoreOS - ELF-64 Binary Parser & Memory Mapping Loader (elf.c)
// =============================================================================

#include "elf.h"
#include "vmm.h"

void kprint(const char* data);
void kprint_hex(uint64_t val);
void kmemcpy(void* dest, const void* src, uint32_t count);

int elf_validate(const void* buffer, uint64_t size) {
    if (!buffer || size < sizeof(Elf64_Ehdr)) return -1;

    const Elf64_Ehdr* header = (const Elf64_Ehdr*)buffer;

    // Check ELF magic bytes (0x7F, 'E', 'L', 'F')
    if (header->e_ident[0] != ELF_MAGIC0 ||
        header->e_ident[1] != ELF_MAGIC1 ||
        header->e_ident[2] != ELF_MAGIC2 ||
        header->e_ident[3] != ELF_MAGIC3) {
        return -2; // Not an ELF binary
    }

    // Verify 64-bit architecture and x86_64 machine type
    if (header->e_ident[4] != ELFCLASS64) return -3;
    if (header->e_machine != EM_X86_64) return -4;

    return 0; // Valid x86_64 ELF binary
}

// Parses ELF headers, creates a dedicated user address space, maps PT_LOAD segments,
// and returns the executable entry point virtual address (RIP)
uint64_t elf_load(const void* buffer, uint64_t size, uint64_t* out_pml4) {
    if (elf_validate(buffer, size) != 0) return 0;

    const Elf64_Ehdr* header = (const Elf64_Ehdr*)buffer;

    // 1. Create a clean, isolated User-Space PML4 Address Space
    pml4_t* user_pml4 = vmm_create_user_space();
    if (!user_pml4) return 0;

    const uint8_t* raw_bytes = (const uint8_t*)buffer;
    const Elf64_Phdr* ph_table = (const Elf64_Phdr*)(raw_bytes + header->e_phoff);

    // 2. Iterate Program Headers and map PT_LOAD segments
    for (uint16_t i = 0; i < header->e_phnum; i++) {
        const Elf64_Phdr* ph = &ph_table[i];

        if (ph->p_type == PT_LOAD) {
            uint64_t vaddr = ph->p_vaddr;
            uint64_t memsz = ph->p_memsz;
            uint64_t filesz = ph->p_filesz;
            uint64_t offset = ph->p_offset;

            // Compute page-aligned bounds
            uint64_t start_page = vaddr & ~0xFFFULL;
            uint64_t end_page = (vaddr + memsz + PAGE_SIZE - 1) & ~0xFFFULL;

            for (uint64_t page = start_page; page < end_page; page += PAGE_SIZE) {
                uint64_t phys_frame = pmm_alloc_frame();
                if (!phys_frame) return 0;

                // Determine segment permissions
                uint64_t flags = PTE_USER | PTE_PRESENT;
                if (ph->p_flags & PF_W) flags |= PTE_WRITABLE;
                if (!(ph->p_flags & PF_X)) flags |= PTE_NX;

                // Map into isolated user address space
                vmm_map_page(user_pml4, page, phys_frame, flags);

                // Copy file data into the allocated frame
                uint64_t page_offset_in_segment = page - start_page;
                if (page_offset_in_segment < filesz) {
                    uint32_t copy_bytes = PAGE_SIZE;
                    if (page_offset_in_segment + copy_bytes > filesz) {
                        copy_bytes = (uint32_t)(filesz - page_offset_in_segment);
                    }
                    kmemcpy((void*)phys_frame, raw_bytes + offset + page_offset_in_segment, copy_bytes);
                }
            }
        }
    }

    // 3. Allocate user stack (mapped at 0x00007FFFFFFFF000, 16KB stack)
    for (uint64_t s = 0; s < 4; s++) {
        uint64_t stack_phys = pmm_alloc_frame();
        vmm_map_page(user_pml4, 0x00007FFFFFFFF000 - (s * PAGE_SIZE), stack_phys, PTE_USER | PTE_WRITABLE | PTE_PRESENT);
    }

    if (out_pml4) {
        *out_pml4 = (uint64_t)user_pml4;
    }

    return header->e_entry; // Return entry point address
}
