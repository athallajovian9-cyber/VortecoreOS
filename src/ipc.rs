//! VortecoreOS - Lock-Free Single Producer Single Consumer (SPSC) Ring Buffer IPC
//! High-throughput microkernel message passing with atomic synchronization.

use core::sync::atomic::{AtomicUsize, Ordering};

pub const IPC_PAYLOAD_SIZE: usize = 64;
pub const RING_SIZE: usize = 16;

#[derive(Clone, Copy)]
pub struct IpcMessage {
    pub sender: u32,
    pub target: u32,
    pub cap_token: u32,
    pub len: usize,
    pub payload: [u8; IPC_PAYLOAD_SIZE],
}

impl IpcMessage {
    pub const fn empty() -> Self {
        Self {
            sender: 0,
            target: 0,
            cap_token: 0,
            len: 0,
            payload: [0; IPC_PAYLOAD_SIZE],
        }
    }
}

pub struct SpscRingBuffer {
    buffer: [IpcMessage; RING_SIZE],
    head: AtomicUsize,
    tail: AtomicUsize,
}

impl SpscRingBuffer {
    pub const fn new() -> Self {
        Self {
            buffer: [IpcMessage::empty(); RING_SIZE],
            head: AtomicUsize::new(0),
            tail: AtomicUsize::new(0),
        }
    }

    pub fn send(&mut self, msg: &IpcMessage) -> Result<(), &'static str> {
        let current_tail = self.tail.load(Ordering::Relaxed);
        let next_tail = (current_tail + 1) % RING_SIZE;

        if next_tail == self.head.load(Ordering::Acquire) {
            return Err("Queue full");
        }

        self.buffer[current_tail] = *msg;
        self.tail.store(next_tail, Ordering::Release);
        Ok(())
    }

    pub fn recv(&mut self) -> Option<IpcMessage> {
        let current_head = self.head.load(Ordering::Relaxed);

        if current_head == self.tail.load(Ordering::Acquire) {
            return None; // Empty
        }

        let msg = self.buffer[current_head];
        let next_head = (current_head + 1) % RING_SIZE;
        self.head.store(next_head, Ordering::Release);
        Some(msg)
    }
}
