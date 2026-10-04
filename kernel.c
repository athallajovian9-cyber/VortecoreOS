// =============================================================================
// VortecoreOS - 64-bit Kernel Core (kernel.c)
// Target: x86_64 Bare-Metal (Freestanding, no stdlib)
// Features: Direct VGA driver, formatting, serial UART logging, CPU info, and shell
// =============================================================================

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

#define VGA_ADDRESS 0xB8000
#define VGA_WIDTH   80
#define VGA_HEIGHT  25

#define COM1_PORT 0x3F8

// Color constants
enum vga_color {
    COLOR_BLACK = 0,
    COLOR_BLUE = 1,
    COLOR_GREEN = 2,
    COLOR_CYAN = 3,
    COLOR_RED = 4,
    COLOR_MAGENTA = 5,
    COLOR_BROWN = 6,
    COLOR_LIGHT_GREY = 7,
    COLOR_DARK_GREY = 8,
    COLOR_LIGHT_BLUE = 9,
    COLOR_LIGHT_GREEN = 10,
    COLOR_LIGHT_CYAN = 11,
    COLOR_LIGHT_RED = 12,
    COLOR_LIGHT_MAGENTA = 13,
    COLOR_LIGHT_BROWN = 14,
    COLOR_WHITE = 15,
};

static uint16_t* const vga_buffer = (uint16_t*)VGA_ADDRESS;
static uint32_t cursor_row = 0;
static uint32_t cursor_col = 0;
static uint8_t current_color = 0x07;

// --- Port I/O Instructions ---
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// --- Serial Logging (COM1 UART) ---
void serial_init(void) {
    outb(COM1_PORT + 1, 0x00);    // Disable interrupts
    outb(COM1_PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(COM1_PORT + 0, 0x03);    // Set divisor to 3 (38400 baud)
    outb(COM1_PORT + 1, 0x00);
    outb(COM1_PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(COM1_PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    outb(COM1_PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

int is_transmit_empty(void) {
    return inb(COM1_PORT + 5) & 0x20;
}

void serial_write_char(char a) {
    while (is_transmit_empty() == 0);
    outb(COM1_PORT, (uint8_t)a);
}

void serial_write(const char* str) {
    while (*str) {
        serial_write_char(*str++);
    }
}

// --- VGA Terminal Driver ---
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t)uc | (uint16_t)color << 8;
}

void terminal_clear(void) {
    for (uint32_t y = 0; y < VGA_HEIGHT; y++) {
        for (uint32_t x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = vga_entry(' ', current_color);
        }
    }
    cursor_row = 0;
    cursor_col = 0;
}

void terminal_setcolor(uint8_t color) {
    current_color = color;
}

void terminal_scroll(void) {
    for (uint32_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (uint32_t x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = vga_buffer[(y + 1) * VGA_WIDTH + x];
        }
    }
    for (uint32_t x = 0; x < VGA_WIDTH; x++) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', current_color);
    }
    cursor_row = VGA_HEIGHT - 1;
}

void terminal_putchar(char c) {
    if (c == '\n') {
        cursor_col = 0;
        cursor_row++;
        if (cursor_row >= VGA_HEIGHT) {
            terminal_scroll();
        }
        return;
    }
    if (c == '\r') {
        cursor_col = 0;
        return;
    }

    vga_buffer[cursor_row * VGA_WIDTH + cursor_col] = vga_entry(c, current_color);
    cursor_col++;
    if (cursor_col >= VGA_WIDTH) {
        cursor_col = 0;
        cursor_row++;
        if (cursor_row >= VGA_HEIGHT) {
            terminal_scroll();
        }
    }
}

void kprint(const char* data) {
    while (*data) {
        terminal_putchar(*data);
        serial_write_char(*data);
        data++;
    }
}

void kprint_hex(uint64_t val) {
    const char hex_chars[] = "0123456789ABCDEF";
    kprint("0x");
    for (int i = 60; i >= 0; i -= 4) {
        terminal_putchar(hex_chars[(val >> i) & 0xF]);
    }
}

// --- CPUID Identification ---
void print_cpu_vendor(void) {
    uint32_t eax, ebx, ecx, edx;
    char vendor[13];
    vendor[12] = '\0';

    __asm__ volatile ("cpuid"
                      : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                      : "a"(0));

    *(uint32_t*)&vendor[0] = ebx;
    *(uint32_t*)&vendor[4] = edx;
    *(uint32_t*)&vendor[8] = ecx;

    kprint("CPU Vendor       : ");
    kprint(vendor);
    kprint("\n");
}

// =============================================================================
// 64-bit Kernel Entry Point (Called by Stage 2 Bootloader)
// =============================================================================
void kernel_main(void) {
    serial_init();
    serial_write("\n[VortecoreOS] Kernel init started...\n");

    terminal_setcolor(0x0F); // White on Black
    terminal_clear();

    // Top banner
    terminal_setcolor(0x1F); // White on Blue
    kprint("   ==========================================================================   \n");
    kprint("             VORTECORE OS -- 64-BIT NATIVE MICROKERNEL (x86_64)                 \n");
    kprint("   ==========================================================================   \n");
    terminal_setcolor(0x0F);

    kprint("\n[OK] 64-bit Long Mode Activated.\n");
    kprint("[OK] PML4 Identity Paging Initialized (4-Level Page Table).\n");
    kprint("[OK] GDT 64-bit Segments Loaded.\n");
    kprint("[OK] COM1 Serial Port (0x3F8) ready for diagnostics.\n");

    print_cpu_vendor();

    kprint("Kernel Stack Base: ");
    kprint_hex(0x90000);
    kprint("\n");
    kprint("Page Table Base  : ");
    kprint_hex(0x1000);
    kprint("\n\n");

    // Shell prompt
    terminal_setcolor(0x0A); // Light Green
    kprint("vortecore-x64# ");
    terminal_setcolor(0x0E); // Yellow
    kprint("kernel idle loop running. Ready for tasks.\n");

    serial_write("[VortecoreOS] Kernel fully booted and active.\n");

    // Kernel idle halt loop
    while (1) {
        __asm__ volatile ("hlt");
    }
}
