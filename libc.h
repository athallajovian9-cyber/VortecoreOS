// =============================================================================
// VortecoreOS - Minimal Standard C Library (libc.h)
// User-space abstractions for Ring 3 applications over raw SYSCALL instructions
// =============================================================================

#ifndef LIBC_H
#define LIBC_H

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;
typedef long long          int64_t;
typedef uint64_t           size_t;

#define NULL ((void*)0)

// I/O & Console
int printf(const char* format, ...);
void putchar(char c);
char getchar(void);
void puts(const char* str);

// Memory Management
void* malloc(size_t size);
void free(void* ptr);
void* memset(void* dest, int val, size_t count);
void* memcpy(void* dest, const void* src, size_t count);

// String Operations
size_t strlen(const char* s);
int strcmp(const char* s1, const char* s2);
char* strcpy(char* dest, const char* src);

// File Operations (Virtual File System / RAMFS via syscalls)
int open(const char* filename, int flags);
int read(const char* filename, void* buf, size_t count);
int write(const char* filename, const void* buf, size_t count);
int remove(const char* filename);

// Process Lifecycle
void exit(int status) __attribute__((noreturn));
uint64_t get_time(void);

#endif
