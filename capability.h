// =============================================================================
// VortecoreOS - Capability-Based Security System (capability.h)
// Architecture: Object-Oriented Capabilities (seL4 / Fuchsia Zircon model)
// Concept: Zero ambient authority. Tokens (64-bit caps) grant granular rights.
// =============================================================================

#ifndef CAPABILITY_H
#define CAPABILITY_H

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

// Capability Rights Bitmask
#define CAP_RIGHT_READ      (1 << 0)
#define CAP_RIGHT_WRITE     (1 << 1)
#define CAP_RIGHT_EXECUTE   (1 << 2)
#define CAP_RIGHT_GRANT     (1 << 3)  // Transfer cap to another process
#define CAP_RIGHT_IPC_SEND  (1 << 4)
#define CAP_RIGHT_IPC_RECV  (1 << 5)
#define CAP_RIGHT_HARDWARE  (1 << 6)  // Driver hardware access right

// Capability Object Types
typedef enum {
    CAP_OBJ_NULL = 0,
    CAP_OBJ_MEMORY,     // Specific memory page frame
    CAP_OBJ_IPC_PORT,   // IPC endpoint
    CAP_OBJ_FILE,       // Storage block / file
    CAP_OBJ_DRIVER,     // User-space hardware driver control
    CAP_OBJ_THREAD,     // Process execution context
} cap_type_t;

// 64-bit Unforgeable Capability Token
typedef struct {
    uint32_t id;            // Kernel-managed secure token ID
    cap_type_t type;        // Resource target
    uint32_t rights;        // Granular permission bitmask
    uint64_t target_object; // Physical resource pointer or ID
    uint32_t owner_pid;     // Process owning this token
    uint8_t  active;
} capability_t;

#define MAX_CAPS 64

void cap_init(void);
capability_t* cap_issue(uint32_t pid, cap_type_t type, uint32_t rights, uint64_t target_obj);
int cap_verify(uint32_t cap_id, uint32_t pid, uint32_t requested_right);
void cap_revoke(uint32_t cap_id);

#endif
