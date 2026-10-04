// =============================================================================
// VortecoreOS - Complete All-Rounder OS Subsystem Registry (subsystems.h)
// Combines: Gaming (Direct LFB & Raw Mouse), Dev (C/ASM/Rust Libc),
//           Server/HFT (Lock-Free IPC & RTOS), Security (Linear Capabilities)
// =============================================================================

#ifndef SUBSYSTEMS_H
#define SUBSYSTEMS_H

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

// All-Rounder Capability Pillars
typedef enum {
    MODE_BALANCED = 0,   // Everyday desktop productivity
    MODE_GAMING,         // Maximum hardware direct access, uncapped LFB, raw mouse
    MODE_DEVELOPER,      // Debugging symbols, POSIX libc syscall tracing, ELF loader
    MODE_SERVER_RTOS,    // Strict O(1) determinism, 0.00 ns jitter, lock-free IPC
    MODE_HARDENED,       // Strict capability enforcement, zero ambient authority
} os_profile_t;

typedef struct {
    os_profile_t active_mode;
    uint32_t cpu_cores_detected;
    uint64_t uptime_ticks;
    uint32_t fps_counter;
    uint32_t active_threads;
    uint64_t free_ram_kb;
    uint8_t  gpu_accel_enabled;
    uint8_t  audio_dsp_active;
    uint8_t  net_stack_active;
} os_state_t;

void subsystems_init(void);
void set_os_profile(os_profile_t mode);
os_state_t* get_os_state(void);

#endif
