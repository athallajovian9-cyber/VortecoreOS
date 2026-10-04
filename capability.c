// =============================================================================
// VortecoreOS - Capability-Based Security Implementation (capability.c)
// =============================================================================

#include "capability.h"

static capability_t cap_table[MAX_CAPS];
static uint32_t next_cap_id = 1000;

void cap_init(void) {
    for (int i = 0; i < MAX_CAPS; i++) {
        cap_table[i].active = 0;
    }
}

capability_t* cap_issue(uint32_t pid, cap_type_t type, uint32_t rights, uint64_t target_obj) {
    for (int i = 0; i < MAX_CAPS; i++) {
        if (!cap_table[i].active) {
            cap_table[i].id = next_cap_id++;
            cap_table[i].type = type;
            cap_table[i].rights = rights;
            cap_table[i].target_object = target_obj;
            cap_table[i].owner_pid = pid;
            cap_table[i].active = 1;
            return &cap_table[i];
        }
    }
    return 0; // Cap table exhausted
}

int cap_verify(uint32_t cap_id, uint32_t pid, uint32_t requested_right) {
    for (int i = 0; i < MAX_CAPS; i++) {
        if (cap_table[i].active && cap_table[i].id == cap_id) {
            // Verify ownership
            if (cap_table[i].owner_pid != pid) return -1; // Unauthorized process
            // Verify granular right bitmask
            if ((cap_table[i].rights & requested_right) == requested_right) {
                return 0; // Granted
            }
            return -2; // Rights insufficient
        }
    }
    return -3; // Invalid capability token
}

void cap_revoke(uint32_t cap_id) {
    for (int i = 0; i < MAX_CAPS; i++) {
        if (cap_table[i].active && cap_table[i].id == cap_id) {
            cap_table[i].active = 0;
            return;
        }
    }
}
