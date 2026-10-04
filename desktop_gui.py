"""VortecoreOS - Graphical Desktop Environment & Window Manager Simulator.
Demonstrates 1024x768 32-bit TrueColor desktop with movable floating windows,
modern taskbar, interactive apps, and terminal emulator.
"""
from __future__ import annotations

import sys
import tkinter as tk
from pathlib import Path

HERE = Path(__file__).resolve().parent


class WindowWidget:
    def __init__(self, parent_canvas, x, y, width, height, title, bg_color="#161B26", is_active=True):
        self.canvas = parent_canvas
        self.x = x
        self.y = y
        self.w = width
        self.h = height
        self.title = title
        self.bg_color = bg_color
        self.is_active = is_active
        self.drag_start = None
        self.tag = f"win_{id(self)}"

    def render(self):
        self.canvas.delete(self.tag)

        # Drop shadow
        self.canvas.create_rectangle(
            self.x + 8, self.y + 8, self.x + self.w + 8, self.y + self.h + 8,
            fill="#05070A", outline="", tags=self.tag
        )

        # Window body
        self.canvas.create_rectangle(
            self.x, self.y, self.x + self.w, self.y + self.h,
            fill=self.bg_color, outline="#2C3549", width=1, tags=self.tag
        )

        # Title bar (34px)
        tb_color = "#22C55E" if self.is_active else "#2D3748"
        self.canvas.create_rectangle(
            self.x, self.y, self.x + self.w, self.y + 34,
            fill=tb_color, outline="", tags=self.tag
        )

        # Title text
        self.canvas.create_text(
            self.x + 14, self.y + 17,
            text=self.title, fill="#0F172A" if self.is_active else "#E2E8F0",
            font=("Segoe UI", 10, "bold"), anchor="w", tags=self.tag
        )

        # Window control dots (Close, Min, Max)
        dot_y = self.y + 17
        self.canvas.create_oval(self.x + self.w - 24, dot_y - 6, self.x + self.w - 12, dot_y + 6, fill="#EF4444", outline="", tags=self.tag)
        self.canvas.create_oval(self.x + self.w - 44, dot_y - 6, self.x + self.w - 32, dot_y + 6, fill="#EAB308", outline="", tags=self.tag)
        self.canvas.create_oval(self.x + self.w - 64, dot_y - 6, self.x + self.w - 52, dot_y + 6, fill="#10B981", outline="", tags=self.tag)


class VortecoreDesktop(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("VortecoreOS Desktop Environment (1024x768 32-bit LFB)")
        self.geometry("1024x768")
        self.configure(bg="#0B0E14")
        self.resizable(False, False)

        self.canvas = tk.Canvas(self, width=1024, height=768, bg="#0B0E14", highlightthickness=0)
        self.canvas.pack(fill=tk.BOTH, expand=True)

        self.windows = [
            WindowWidget(self.canvas, 60, 60, 520, 360, "⚡ Vortecore Terminal (x86_64)", "#0F121A", True),
            WindowWidget(self.canvas, 460, 180, 500, 380, "📊 RTOS Real-Time Monitor & Benchmark", "#141824", False),
            WindowWidget(self.canvas, 140, 380, 420, 260, "📁 RAMFS File Explorer", "#131620", False),
        ]

        self.active_win_idx = 0
        self.dragged_win = None
        self.drag_offset_x = 0
        self.drag_offset_y = 0

        self.canvas.bind("<Button-1>", self._on_click)
        self.canvas.bind("<B1-Motion>", self._on_drag)
        self.canvas.bind("<ButtonRelease-1>", self._on_release)

        self._render_desktop()

    def _render_desktop(self):
        self.canvas.delete("all")

        # Desktop wallpaper gradient grid
        for y in range(0, 720, 36):
            for x in range(0, 1024, 36):
                self.canvas.create_rectangle(x, y, x + 36, y + 36, fill="#0B0E14", outline="#131822")

        # Render floating windows
        for i, win in enumerate(self.windows):
            win.is_active = (i == self.active_win_idx)
            win.render()
            self._render_window_contents(win)

        # Render modern bottom Taskbar (48px)
        self.canvas.create_rectangle(0, 720, 1024, 768, fill="#12161F", outline="#222938")

        # Start button pill
        self.canvas.create_rectangle(14, 728, 130, 760, fill="#22C55E", outline="")
        self.canvas.create_text(72, 744, text="VORTECORE", fill="#0A0F14", font=("Segoe UI", 9, "bold"))

        # Taskbar window tabs
        for idx, win in enumerate(self.windows):
            tx = 145 + idx * 160
            t_color = "#222C3D" if idx == self.active_win_idx else "#161C26"
            self.canvas.create_rectangle(tx, 728, tx + 150, 760, fill=t_color, outline="#2C3549")
            self.canvas.create_text(tx + 12, 744, text=win.title[:18], fill="#FFFFFF", font=("Segoe UI", 8), anchor="w")

        # Clock & System Tray
        self.canvas.create_text(960, 744, text="64-bit  2026", fill="#38BDF8", font=("Consolas", 9, "bold"))

    def _render_window_contents(self, win: WindowWidget):
        # Render Terminal inside Window 0
        if "Terminal" in win.title:
            self.canvas.create_text(
                win.x + 18, win.y + 55,
                text="VortecoreOS 64-bit Kernel v1.2.0 (x86_64)\n"
                     "Microkernel active · RAMFS mounted · Paging 4-Level\n\n"
                     "vortecore-rust# moglinux\n"
                     "[MOG] Context switch: 18 cycles (Linux: 1200+)\n"
                     "[MOG] Zero ambient authority capabilities active.\n"
                     "vortecore-rust# _",
                fill="#38BDF8", font=("Consolas", 9), anchor="nw", tags=win.tag
            )
        # Render RTOS monitor inside Window 1
        elif "Monitor" in win.title:
            self.canvas.create_text(
                win.x + 18, win.y + 55,
                text="REAL-TIME HARDWARE & THREAD METRICS\n"
                     "--------------------------------------------\n"
                     "Scheduler  : O(1) Preemptive Static Priority RTOS\n"
                     "Jitter     : 0.00 ns (Hard Real-Time Guaranteed)\n"
                     "IPC Queue  : SPSC Lock-Free Ring Buffer (0 Locks)\n"
                     "Active PID : 1 [Avionics Flight Control - Prio 0]\n"
                     "Active PID : 2 [User NVMe Driver - Prio 1]\n"
                     "Security   : 100% Ring 3 Isolation (DPL 3)\n"
                     "Crash Guard: Self-Healing Microkernel Active",
                fill="#10B981", font=("Consolas", 9), anchor="nw", tags=win.tag
            )
        # Render File Explorer in Window 2
        elif "RAMFS" in win.title:
            files = ["📄 readme.txt (112 B)", "⚙️ version.sys (48 B)", "📜 motd (42 B)", "🚀 hello.elf (1.2 KB)"]
            for f_idx, f_name in enumerate(files):
                fy = win.y + 60 + f_idx * 30
                self.canvas.create_text(win.x + 20, fy, text=f_name, fill="#F8FAFC", font=("Segoe UI", 9), anchor="w", tags=win.tag)

    def _on_click(self, event):
        # Check if clicked inside a window
        for idx in range(len(self.windows) - 1, -1, -1):
            win = self.windows[idx]
            if win.x <= event.x <= win.x + win.w and win.y <= event.y <= win.y + win.h:
                # Bring window to front
                clicked_win = self.windows.pop(idx)
                self.windows.append(clicked_win)
                self.active_win_idx = len(self.windows) - 1

                # If clicked titlebar, initiate drag
                if event.y <= clicked_win.y + 34:
                    self.dragged_win = clicked_win
                    self.drag_offset_x = event.x - clicked_win.x
                    self.drag_offset_y = event.y - clicked_win.y

                self._render_desktop()
                return

    def _on_drag(self, event):
        if self.dragged_win:
            self.dragged_win.x = max(0, min(1024 - self.dragged_win.w, event.x - self.drag_offset_x))
            self.dragged_win.y = max(0, min(720 - self.dragged_win.h, event.y - self.drag_offset_y))
            self._render_desktop()

    def _on_release(self, _event):
        self.dragged_win = None


if __name__ == "__main__":
    app = VortecoreDesktop()
    app.mainloop()
