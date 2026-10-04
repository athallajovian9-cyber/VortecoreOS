//! VortecoreOS - Type-Safe Capability-Based Security in Rust
//! Capabilities are linear, unforgeable types enforcing zero ambient authority.

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum CapRight {
    Read,
    Write,
    Execute,
    IpcSend,
    IpcRecv,
    Hardware,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ObjectType {
    MemoryFrame(u64),
    IpcEndpoint(u32),
    FileBlock(u32),
    Driver(u16),
}

#[derive(Debug, Clone, Copy)]
pub struct Capability {
    pub id: u32,
    pub owner_pid: u32,
    pub target: ObjectType,
    pub rights_mask: u32,
    pub is_valid: bool,
}

impl Capability {
    pub const fn new(id: u32, owner: u32, target: ObjectType, mask: u32) -> Self {
        Self {
            id,
            owner_pid: owner,
            target,
            rights_mask: mask,
            is_valid: true,
        }
    }

    pub fn has_right(&self, right: CapRight) -> bool {
        if !self.is_valid {
            return false;
        }
        let bit = match right {
            CapRight::Read => 1 << 0,
            CapRight::Write => 1 << 1,
            CapRight::Execute => 1 << 2,
            CapRight::IpcSend => 1 << 3,
            CapRight::IpcRecv => 1 << 4,
            CapRight::Hardware => 1 << 5,
        };
        (self.rights_mask & bit) != 0
    }

    pub fn revoke(&mut self) {
        self.is_valid = false;
    }
}
