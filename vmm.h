#ifndef VMM_H
#define VMM_H

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

typedef uint64_t pml4_t;

#define PAGE_SIZE       4096

#define PTE_PRESENT     (1ULL << 0)
#define PTE_WRITABLE    (1ULL << 1)
#define PTE_USER        (1ULL << 2)
#define PTE_NX          (1ULL << 63)

void vmm_init(void);
uint64_t pmm_alloc_frame(void);
void pmm_free_frame(uint64_t phys_addr);
int vmm_map_page(pml4_t* pml4, uint64_t virt_addr, uint64_t phys_addr, uint64_t flags);
pml4_t* vmm_create_user_space(void);
void vmm_switch_space(pml4_t* pml4);
uint64_t vmm_get_free_ram_kb(void);

#endif
