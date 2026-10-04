// =============================================================================
// VortecoreOS - All-Rounder Architecture Implementation (subsystems.c)
// =============================================================================

#include "subsystems.h"

static os_state_t global_state;

void subsystems_init(void) {
    global_state.active_mode = MODE_BALANCED;
    global_state.cpu_cores_detected = 8;
    global_state.uptime_ticks = 0;
    global_state.fps_counter = 144;      // Uncapped high-refresh LFB
    global_state.active_threads = 4;
    global_state.free_ram_kb = 120832;
    global_state.gpu_accel_enabled = 1;
    global_state.audio_dsp_active = 1;
    global_state.net_stack_active = 1;
}

void set_os_profile(os_profile_t mode) {
    global_state.active_mode = mode;
    switch (mode) {
        case MODE_GAMING:
            global_state.fps_counter = 240;
            global_state.gpu_accel_enabled = 1;
            break;
        case MODE_DEVELOPER:
            global_state.active_threads = 12;
            break;
        case MODE_SERVER_RTOS:
            global_state.fps_counter = 60;
            break;
        case MODE_HARDENED:
            break;
        default:
            global_state.fps_counter = 144;
            break;
    }
}

os_state_t* get_os_state(void) {
    return &global_state;
}
