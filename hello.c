// =============================================================================
// VortecoreOS - Sample User-Space Application (hello.c)
// Compiles into a 64-bit ELF executable, runs purely in Ring 3 using libc
// =============================================================================

#include "libc.h"

int main(int argc, char** argv) {
    (void)argc; (void)argv;

    printf("\n[USER APP: hello.elf] Hello from Ring 3 User Space!\n");
    printf("Running on isolated page tables under VortecoreOS.\n");

    // Dynamic memory allocation test via libc malloc()
    char* msg = (char*)malloc(64);
    if (msg) {
        strcpy(msg, "Dynamic heap memory allocated via syscall SYS_ALLOC_MEM!");
        printf("[LIBC MALLOC] Heap test: %s\n", msg);
    }

    // File I/O test via libc read()
    char buf[128];
    int bytes = read("version.sys", buf, 127);
    if (bytes > 0) {
        buf[bytes] = '\0';
        printf("[LIBC VFS] Read 'version.sys': %s", buf);
    }

    uint64_t ticks = get_time();
    printf("[LIBC RDTSC] Current CPU cycle count: %x\n", ticks);

    printf("[USER APP] Exiting cleanly with exit(0)...\n\n");
    exit(0);
}
