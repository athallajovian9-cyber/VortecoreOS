// =============================================================================
// VortecoreOS - Lock-Free SPSC Ring Buffer IPC Implementation (ipc.c)
// =============================================================================

#include "ipc.h"

static ipc_channel_t channels[MAX_IPC_CHANNELS];

void ipc_init(void) {
    for (int i = 0; i < MAX_IPC_CHANNELS; i++) {
        channels[i].head = 0;
        channels[i].tail = 0;
    }
}

// Lock-Free enqueue (Single Producer)
int ipc_send(uint32_t channel_id, const ipc_msg_t* msg) {
    if (channel_id >= MAX_IPC_CHANNELS || !msg) return -1;

    ipc_channel_t* chan = &channels[channel_id];
    uint32_t current_tail = chan->tail;
    uint32_t next_tail = (current_tail + 1) % IPC_RING_BUFFER_SZ;

    // Check if buffer is full
    if (next_tail == chan->head) {
        return -2; // Queue full
    }

    // Copy message packet
    chan->buffer[current_tail] = *msg;

    // Memory barrier before updating tail pointer
    __asm__ volatile ("" : : : "memory");
    chan->tail = next_tail;
    return 0; // Success
}

// Lock-Free dequeue (Single Consumer)
int ipc_recv(uint32_t channel_id, ipc_msg_t* out_msg) {
    if (channel_id >= MAX_IPC_CHANNELS || !out_msg) return -1;

    ipc_channel_t* chan = &channels[channel_id];
    uint32_t current_head = chan->head;

    // Check if buffer is empty
    if (current_head == chan->tail) {
        return -2; // Queue empty
    }

    // Retrieve message
    *out_msg = chan->buffer[current_head];

    __asm__ volatile ("" : : : "memory");
    chan->head = (current_head + 1) % IPC_RING_BUFFER_SZ;
    return 0; // Success
}
