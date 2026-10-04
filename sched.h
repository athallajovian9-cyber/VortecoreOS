// =============================================================================
// VortecoreOS - Hard Real-Time Deterministic Preemptive Scheduler (sched.h)
// Concept: Priority-based O(1) Preemptive Scheduling with Pit Timer Slicing
// =============================================================================

#ifndef SCHED_H
#define SCHED_H

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

typedef enum {
    THREAD_READY = 0,
    THREAD_RUNNING,
    THREAD_BLOCKED,
    THREAD_TERMINATED,
} thread_state_t;

// Priority classes (Hard RTOS guarantees highest priority runs immediately)
#define PRIORITY_REALTIME  0    // Aerospace / Robotics / Sub-microsecond tasks
#define PRIORITY_DRIVER    1    // User-space drivers (storage, network)
#define PRIORITY_NORMAL    2    // Standard user-space applications
#define PRIORITY_IDLE      3    // Background idle worker

#define MAX_THREADS 16

typedef struct {
    uint32_t pid;
    uint32_t priority;
    thread_state_t state;
    uint64_t rsp;              // Saved 64-bit stack pointer
    uint64_t cr3;              // Isolated Page Directory Base (PML4)
    uint64_t time_slice_ticks; // Preemption time budget
    uint64_t total_cycles;
    char name[24];
} thread_t;

void sched_init(void);
int sched_create_thread(const char* name, uint32_t priority, uint64_t rip, uint64_t cr3);
void sched_tick(void); // Called on hardware PIT timer interrupt (IRQ0)
thread_t* sched_current_thread(void);

#endif
