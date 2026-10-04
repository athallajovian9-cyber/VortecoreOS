// =============================================================================
// VortecoreOS - Deterministic Preemptive RTOS Scheduler Implementation (sched.c)
// =============================================================================

#include "sched.h"

static thread_t threads[MAX_THREADS];
static uint32_t current_thread_idx = 0;
static uint32_t total_threads = 0;

void kstrcpy(char* dest, const char* src);

void sched_init(void) {
    for (int i = 0; i < MAX_THREADS; i++) {
        threads[i].state = THREAD_TERMINATED;
    }
    total_threads = 0;
    current_thread_idx = 0;

    // Thread 0: Kernel Idle Thread
    threads[0].pid = 0;
    threads[0].priority = PRIORITY_IDLE;
    threads[0].state = THREAD_RUNNING;
    threads[0].cr3 = 0x1000; // Kernel PML4
    threads[0].time_slice_ticks = 10;
    kstrcpy(threads[0].name, "kernel_idle");
    total_threads = 1;
}

int sched_create_thread(const char* name, uint32_t priority, uint64_t rip, uint64_t cr3) {
    for (int i = 1; i < MAX_THREADS; i++) {
        if (threads[i].state == THREAD_TERMINATED) {
            threads[i].pid = i;
            threads[i].priority = priority;
            threads[i].state = THREAD_READY;
            threads[i].cr3 = cr3;
            threads[i].time_slice_ticks = 5;
            threads[i].total_cycles = 0;
            kstrcpy(threads[i].name, name);
            total_threads++;
            return i;
        }
    }
    return -1; // Process table full
}

// O(1) Deterministic Priority Selection
// Highest priority (lowest numeric value) always preempts lower priority tasks
void sched_tick(void) {
    uint32_t best_thread = 0;
    uint32_t highest_prio = PRIORITY_IDLE;

    for (uint32_t i = 0; i < MAX_THREADS; i++) {
        if (threads[i].state == THREAD_READY || threads[i].state == THREAD_RUNNING) {
            if (threads[i].priority < highest_prio) {
                highest_prio = threads[i].priority;
                best_thread = i;
            }
        }
    }

    if (best_thread != current_thread_idx) {
        // Preempt current thread and switch to best_thread
        threads[current_thread_idx].state = THREAD_READY;
        current_thread_idx = best_thread;
        threads[current_thread_idx].state = THREAD_RUNNING;
    }
}

thread_t* sched_current_thread(void) {
    return &threads[current_thread_idx];
}
