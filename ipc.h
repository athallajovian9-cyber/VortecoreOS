// =============================================================================
// VortecoreOS - Lock-Free Ring Buffer IPC Architecture (ipc.h)
// Architecture: Microkernel Message Passing between isolated Ring 3 services
// =============================================================================

#ifndef IPC_H
#define IPC_H

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

#define IPC_MSG_PAYLOAD_SZ 64
#define IPC_RING_BUFFER_SZ 16

// Structured IPC Message Packet
typedef struct {
    uint32_t sender_pid;
    uint32_t target_pid;
    uint32_t msg_type;
    uint32_t capability_token; // Attached capability token
    uint32_t length;
    uint8_t  payload[IPC_MSG_PAYLOAD_SZ];
} ipc_msg_t;

// Lock-Free Single Producer Single Consumer (SPSC) Ring Buffer
typedef struct {
    volatile uint32_t head;
    volatile uint32_t tail;
    ipc_msg_t buffer[IPC_RING_BUFFER_SZ];
} ipc_channel_t;

#define MAX_IPC_CHANNELS 8

void ipc_init(void);
int ipc_send(uint32_t channel_id, const ipc_msg_t* msg);
int ipc_recv(uint32_t channel_id, ipc_msg_t* out_msg);

#endif
