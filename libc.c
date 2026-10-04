// =============================================================================
// VortecoreOS - Minimal Standard C Library Implementation (libc.c)
// =============================================================================

#include "libc.h"
#include "syscall.h"
#include <stdarg.h>

// User-space heap state
#define HEAP_PAGE_SIZE 4096
static uint8_t* current_heap_break = NULL;
static size_t heap_remaining = 0;

void exit(int status) {
    user_syscall(SYS_EXIT, (uint64_t)status, 0, 0);
    while (1) {}
}

void putchar(char c) {
    char buf[2] = {c, '\0'};
    user_syscall(SYS_PRINT, (uint64_t)buf, 1, 0);
}

void puts(const char* str) {
    user_syscall(SYS_PRINT, (uint64_t)str, strlen(str), 0);
    putchar('\n');
}

char getchar(void) {
    return (char)user_syscall(SYS_READ_KEY, 0, 0, 0);
}

size_t strlen(const char* s) {
    size_t len = 0;
    while (s[len]) len++;
    return len;
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

char* strcpy(char* dest, const char* src) {
    char* orig = dest;
    while (*src) *dest++ = *src++;
    *dest = '\0';
    return orig;
}

void* memset(void* dest, int val, size_t count) {
    uint8_t* d = (uint8_t*)dest;
    while (count--) *d++ = (uint8_t)val;
    return dest;
}

void* memcpy(void* dest, const void* src, size_t count) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    while (count--) *d++ = *s++;
    return dest;
}

// User-space bump allocator over SYS_ALLOC_MEM syscall
void* malloc(size_t size) {
    if (size == 0) return NULL;
    size = (size + 7) & ~7ULL; // 8-byte alignment

    if (heap_remaining < size) {
        uint64_t new_page = user_syscall(SYS_ALLOC_MEM, HEAP_PAGE_SIZE, 0, 0);
        if (new_page == 0) return NULL; // Out of memory
        current_heap_break = (uint8_t*)new_page;
        heap_remaining = HEAP_PAGE_SIZE;
    }

    void* ptr = current_heap_break;
    current_heap_break += size;
    heap_remaining -= size;
    return ptr;
}

void free(void* ptr) {
    (void)ptr; // Bump allocator does not reclaim individual allocations
}

// File I/O abstractions
int read(const char* filename, void* buf, size_t count) {
    return (int)user_syscall(SYS_FS_READ, (uint64_t)filename, (uint64_t)buf, (uint64_t)count);
}

int write(const char* filename, const void* buf, size_t count) {
    return (int)user_syscall(SYS_FS_WRITE, (uint64_t)filename, (uint64_t)buf, (uint64_t)count);
}

int remove(const char* filename) {
    return (int)user_syscall(SYS_FS_DELETE, (uint64_t)filename, 0, 0);
}

uint64_t get_time(void) {
    return (uint64_t)user_syscall(SYS_GET_TIME, 0, 0, 0);
}

// Formatted printf implementation supporting %s, %d, %x, %c
int printf(const char* format, ...) {
    va_list args;
    va_start(args, format);

    int count = 0;
    while (*format) {
        if (*format == '%' && *(format + 1)) {
            format++;
            if (*format == 's') {
                const char* str = va_arg(args, const char*);
                if (!str) str = "(null)";
                user_syscall(SYS_PRINT, (uint64_t)str, strlen(str), 0);
                count += (int)strlen(str);
            }
            else if (*format == 'd') {
                int num = va_arg(args, int);
                if (num == 0) {
                    putchar('0');
                    count++;
                } else {
                    if (num < 0) {
                        putchar('-');
                        num = -num;
                        count++;
                    }
                    char buf[12];
                    int i = 0;
                    while (num > 0) {
                        buf[i++] = (num % 10) + '0';
                        num /= 10;
                    }
                    while (i > 0) {
                        putchar(buf[--i]);
                        count++;
                    }
                }
            }
            else if (*format == 'x') {
                uint64_t hex = va_arg(args, uint64_t);
                const char hex_chars[] = "0123456789abcdef";
                putchar('0'); putchar('x');
                count += 2;
                for (int s = 60; s >= 0; s -= 4) {
                    putchar(hex_chars[(hex >> s) & 0xF]);
                    count++;
                }
            }
            else if (*format == 'c') {
                char ch = (char)va_arg(args, int);
                putchar(ch);
                count++;
            }
            else if (*format == '%') {
                putchar('%');
                count++;
            }
        } else {
            putchar(*format);
            count++;
        }
        format++;
    }

    va_end(args);
    return count;
}
