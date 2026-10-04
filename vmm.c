// =============================================================================
// VortecoreOS - 64-bit Virtual Memory Manager (Paging & Address Space Isolation)
// Architecture: x86_64 4-Level Paging (PML4, PDPT, PD, PT)
// Features: Physical Page Frame Allocator (Bitmap), Address Space Cloning,
//           User-Space Isolation (Ring 3 vs Ring 0 protection bits)
// =============================================================================

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

#define PAGE_SIZE       4096
#define PAGE_ALIGN(x)   (((x) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))

// Page Table Entry (PTE) Flags
#define PTE_PRESENT     (1ULL << 0)
#define PTE_WRITABLE    (1ULL << 1)
#define PTE_USER        (1ULL << 2)  // User-mode (Ring 3) accessible
#define PTE_PWT         (1ULL << 3)  // Write-through
#define PTE_PCD         (1ULL << 4)  // Cache-disabled
#define PTE_ACCESSED    (1ULL << 5)
#define PTE_DIRTY       (1ULL << 6)
#define PTE_HUGE        (1ULL << 7)  // 2MB / 1GB page
#define PTE_GLOBAL      (1ULL << 8)
#define PTE_NX          (1ULL << 63) // No-Execute bit

#define TOTAL_PAGES     32768        // Manages 128MB of physical RAM in 4KB chunks
#define BITMAP_SIZE     (TOTAL_PAGES / 64)

// 64-bit Page Directory Pointer
typedef uint64_t pml4_t;
typedef uint64_t pdpt_t;
typedef uint64_t pd_t;
typedef uint64_t pt_t;

// --- Physical Frame Allocator (Bitmap) ---
static uint64_t page_bitmap[BITMAP_SIZE];
static uint64_t total_free_pages = 0;

static inline void bitmap_set(uint64_t page_idx) {
    page_bitmap[page_idx / 64] |= (1ULL << (page_idx % 64));
}

static inline void bitmap_clear(uint64_t page_idx) {
    page_bitmap[page_idx / 64] &= ~(1ULL << (page_idx % 64));
}

static inline int bitmap_test(uint64_t page_idx) {
    return (page_bitmap[page_idx / 64] & (1ULL << (page_idx % 64))) != 0;
}

// Allocate one physical 4KB frame
uint64_t pmm_alloc_frame(void) {
    for (uint64_t i = 0; i < TOTAL_PAGES; i++) {
        if (!bitmap_test(i)) {
            bitmap_set(i);
            total_free_pages--;
            return i * PAGE_SIZE;
        }
    }
    return 0; // Out of physical memory
}

void pmm_free_frame(uint64_t phys_addr) {
    uint64_t idx = phys_addr / PAGE_SIZE;
    if (bitmap_test(idx)) {
        bitmap_clear(idx);
        total_free_pages++;
    }
}

// Zero-out a memory page
void zero_page(uint64_t phys_addr) {
    uint64_t* ptr = (uint64_t*)phys_addr;
    for (int i = 0; i < 512; i++) {
        ptr[i] = 0;
    }
}

// --- Virtual Memory Manager (VMM) ---
static pml4_t* kernel_pml4 = (pml4_t*)0x1000;

void vmm_init(void) {
    // 1. Initialize physical frame allocator
    for (uint64_t i = 0; i < BITMAP_SIZE; i++) {
        page_bitmap[i] = 0;
    }
    total_free_pages = TOTAL_PAGES;

    // Reserve first 8MB for hardware, BIOS, kernel code, and early tables
    for (uint64_t i = 0; i < (8 * 1024 * 1024) / PAGE_SIZE; i++) {
        bitmap_set(i);
        total_free_pages--;
    }
}

// Get Page Table indices from virtual address
#define PML4_INDEX(va)  (((va) >> 39) & 0x1FF)
#define PDPT_INDEX(va)  (((va) >> 30) & 0x1FF)
#define PD_INDEX(va)    (((va) >> 21) & 0x1FF)
#define PT_INDEX(va)    (((va) >> 12) & 0x1FF)

// Map a 4KB virtual page to a physical address inside a given PML4 table
int vmm_map_page(pml4_t* pml4, uint64_t virt_addr, uint64_t phys_addr, uint64_t flags) {
    uint64_t pml4_i = PML4_INDEX(virt_addr);
    uint64_t pdpt_i = PDPT_INDEX(virt_addr);
    uint64_t pd_i   = PD_INDEX(virt_addr);
    uint64_t pt_i   = PT_INDEX(virt_addr);

    // 1. Traverse or allocate PDPT
    if (!(pml4[pml4_i] & PTE_PRESENT)) {
        uint64_t new_pdpt = pmm_alloc_frame();
        if (!new_pdpt) return -1;
        zero_page(new_pdpt);
        pml4[pml4_i] = new_pdpt | PTE_PRESENT | PTE_WRITABLE | (flags & PTE_USER);
    } else if (flags & PTE_USER) {
        pml4[pml4_i] |= PTE_USER;
    }
    pdpt_t* pdpt = (pdpt_t*)(pml4[pml4_i] & ~0xFFFULL);

    // 2. Traverse or allocate Page Directory (PD)
    if (!(pdpt[pdpt_i] & PTE_PRESENT)) {
        uint64_t new_pd = pmm_alloc_frame();
        if (!new_pd) return -1;
        zero_page(new_pd);
        pdpt[pdpt_i] = new_pd | PTE_PRESENT | PTE_WRITABLE | (flags & PTE_USER);
    } else if (flags & PTE_USER) {
        pdpt[pdpt_i] |= PTE_USER;
    }
    pd_t* pd = (pd_t*)(pdpt[pdpt_i] & ~0xFFFULL);

    // 3. Traverse or allocate Page Table (PT)
    if (!(pd[pd_i] & PTE_PRESENT)) {
        uint64_t new_pt = pmm_alloc_frame();
        if (!new_pt) return -1;
        zero_page(new_pt);
        pd[pd_i] = new_pt | PTE_PRESENT | PTE_WRITABLE | (flags & PTE_USER);
    } else if (flags & PTE_USER) {
        pd[pd_i] |= PTE_USER;
    }
    pt_t* pt = (pt_t*)(pd[pd_i] & ~0xFFFULL);

    // 4. Map the physical page in the Page Table
    pt[pt_i] = (phys_addr & ~0xFFFULL) | flags | PTE_PRESENT;

    // Invalidate TLB for this virtual address
    __asm__ volatile ("invlpg (%0)" : : "r"(virt_addr) : "memory");
    return 0;
}

// Create an isolated User Address Space (Clones Kernel mappings above 2GB, isolates User space)
pml4_t* vmm_create_user_space(void) {
    uint64_t user_pml4_phys = pmm_alloc_frame();
    if (!user_pml4_phys) return 0;
    zero_page(user_pml4_phys);

    pml4_t* user_pml4 = (pml4_t*)user_pml4_phys;

    // Map Kernel space into User PML4 (PML4[0] covers lower kernel, entries 256-511 cover higher half)
    // Kernel mappings are NOT given PTE_USER flag, so user code accessing them triggers Page Fault (#PF)
    user_pml4[0] = kernel_pml4[0] & ~PTE_USER;

    return user_pml4;
}

// Switch active address space (Load CR3)
void vmm_switch_space(pml4_t* pml4) {
    __asm__ volatile ("mov %0, %%cr3" : : "r"(pml4) : "memory");
}

uint64_t vmm_get_free_ram_kb(void) {
    return (total_free_pages * PAGE_SIZE) / 1024;
}
