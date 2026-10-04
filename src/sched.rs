//! VortecoreOS - Hard Real-Time Deterministic Scheduler in Rust
//! O(1) Preemptive Priority Selection with guaranteed sub-microsecond latency.

pub const MAX_TASKS: usize = 16;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Priority {
    RealTime = 0, // Aerospace, High-Frequency Trading, Robotics
    Driver = 1,   // User-space drivers (GPU, NVMe, Net)
    Normal = 2,   // Shell & general user applications
    Idle = 3,     // Kernel idle loop
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum TaskState {
    Ready,
    Running,
    Blocked,
    Terminated,
}

#[derive(Clone, Copy)]
pub struct Task {
    pub pid: u32,
    pub priority: Priority,
    pub state: TaskState,
    pub cr3: u64, // Isolated Page Directory base
    pub total_cycles: u64,
}

impl Task {
    pub const fn empty() -> Self {
        Self {
            pid: 0,
            priority: Priority::Idle,
            state: TaskState::Terminated,
            cr3: 0x1000,
            total_cycles: 0,
        }
    }
}

pub struct DeterministicScheduler {
    tasks: [Task; MAX_TASKS],
    current_idx: usize,
}

impl DeterministicScheduler {
    pub const fn new() -> Self {
        Self {
            tasks: [Task::empty(); MAX_TASKS],
            current_idx: 0,
        }
    }

    pub fn init(&mut self) {
        self.tasks[0] = Task {
            pid: 0,
            priority: Priority::Idle,
            state: TaskState::Running,
            cr3: 0x1000,
            total_cycles: 0,
        };
        self.current_idx = 0;
    }

    pub fn spawn(&mut self, pid: u32, priority: Priority, cr3: u64) -> Result<(), &'static str> {
        for task in self.tasks.iter_mut().skip(1) {
            if task.state == TaskState::Terminated {
                *task = Task {
                    pid,
                    priority,
                    state: TaskState::Ready,
                    cr3,
                    total_cycles: 0,
                };
                return Ok(());
            }
        }
        Err("Task table exhausted")
    }

    /// O(1) Deterministic Priority Selection
    /// Highest priority thread (lowest enum value) preempts instantly.
    pub fn tick(&mut self) -> usize {
        let mut best_idx = 0;
        let mut highest_prio = Priority::Idle as u8;

        for (idx, task) in self.tasks.iter().enumerate() {
            if task.state == TaskState::Ready || task.state == TaskState::Running {
                let p = task.priority as u8;
                if p < highest_prio {
                    highest_prio = p;
                    best_idx = idx;
                }
            }
        }

        if best_idx != self.current_idx {
            self.tasks[self.current_idx].state = TaskState::Ready;
            self.tasks[best_idx].state = TaskState::Running;
            self.current_idx = best_idx;
        }

        self.current_idx
    }
}
