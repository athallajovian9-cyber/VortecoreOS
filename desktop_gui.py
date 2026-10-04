"""VortecoreOS - Graphical Desktop Environment & Window Manager Simulator.
Demonstrates 1024x768 32-bit TrueColor desktop with movable floating windows,
modern taskbar, interactive apps, and terminal emulator.
"""
from __future__ import annotations

import sys
import tkinter as tk
from pathlib import Path

HERE = Path(__file__).resolve().parent

THEMES = {
    "Cyberpunk": {
        "bg": "#0A0A10",
        "grid": "#1A1528",
        "taskbar": "#12101C",
        "win_bg": "#151420",
        "border": "#3B2D54",
        "titlebar_act": "#A855F7",
        "titlebar_ina": "#2D2640",
        "accent": "#F43F5E",
        "text": "#FDF4FF",
        "tag": "Cyberpunk Neon 240Hz",
    },
    "Matrix": {
        "bg": "#020804",
        "grid": "#081F0E",
        "taskbar": "#041008",
        "win_bg": "#06140A",
        "border": "#0D3315",
        "titlebar_act": "#16A34A",
        "titlebar_ina": "#0F2914",
        "accent": "#22C55E",
        "text": "#DCFCE7",
        "tag": "Matrix Terminal 165Hz",
    },
    "Nord": {
        "bg": "#2E3440",
        "grid": "#3B4252",
        "taskbar": "#282C34",
        "win_bg": "#3B4252",
        "border": "#4C566A",
        "titlebar_act": "#88C0D0",
        "titlebar_ina": "#434C5E",
        "accent": "#81A1C1",
        "text": "#ECEFF4",
        "tag": "Nordic Frost 144Hz",
    },
    "Solarized": {
        "bg": "#002B36",
        "grid": "#073642",
        "taskbar": "#00212B",
        "win_bg": "#073642",
        "border": "#0A4656",
        "titlebar_act": "#268BD2",
        "titlebar_ina": "#002B36",
        "accent": "#B58900",
        "text": "#FDF6E3",
        "tag": "Solarized Ocean 120Hz",
    },
    "Obsidian": {
        "bg": "#0B0E14",
        "grid": "#131822",
        "taskbar": "#12161F",
        "win_bg": "#161B26",
        "border": "#2C3549",
        "titlebar_act": "#22C55E",
        "titlebar_ina": "#2D3748",
        "accent": "#38BDF8",
        "text": "#F8FAFC",
        "tag": "Obsidian Deep 144Hz",
    },
}


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

    def render(self, theme):
        self.canvas.delete(self.tag)

        # Drop shadow
        self.canvas.create_rectangle(
            self.x + 8, self.y + 8, self.x + self.w + 8, self.y + self.h + 8,
            fill="#030406", outline="", tags=self.tag
        )

        # Window body
        self.canvas.create_rectangle(
            self.x, self.y, self.x + self.w, self.y + self.h,
            fill=theme["win_bg"], outline=theme["border"], width=1, tags=self.tag
        )

        # Title bar (34px)
        tb_color = theme["titlebar_act"] if self.is_active else theme["titlebar_ina"]
        self.canvas.create_rectangle(
            self.x, self.y, self.x + self.w, self.y + 34,
            fill=tb_color, outline="", tags=self.tag
        )

        # Title text
        self.canvas.create_text(
            self.x + 14, self.y + 17,
            text=self.title, fill="#FFFFFF" if self.is_active else "#94A3B8",
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
        self.title("VortecoreOS - 100% Fully Customizable Desktop Environment (x86_64)")
        self.geometry("1040x780")
        self.configure(bg="#0B0E14")
        self.resizable(False, False)

        # User Profile & Identity
        self.current_user = "Athalla"
        self.current_hostname = "vortecore-rig"
        self.current_theme_name = "Cyberpunk"

        self.canvas = tk.Canvas(self, width=1040, height=780, bg="#0B0E14", highlightthickness=0)
        self.canvas.pack(fill=tk.BOTH, expand=True)

        self.windows = [
            WindowWidget(self.canvas, 50, 50, 480, 340, "🎨 OS Customizer & Theme Studio", "#151420", True),
            WindowWidget(self.canvas, 500, 100, 500, 360, "⚡ Vortecore Terminal (x86_64)", "#0F121A", False),
            WindowWidget(self.canvas, 100, 380, 480, 300, "📊 Hardware & Real-Time Monitor", "#141824", False),
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
        theme = THEMES[self.current_theme_name]

        # 1. Desktop wallpaper grid
        for y in range(0, 730, 36):
            for x in range(0, 1040, 36):
                self.canvas.create_rectangle(x, y, x + 36, y + 36, fill=theme["bg"], outline=theme["grid"])

        # 2. Render floating windows
        for i, win in enumerate(self.windows):
            win.is_active = (i == self.active_win_idx)
            win.render(theme)
            self._render_window_contents(win, theme)

        # 3. Modern Taskbar (Height 50px at bottom)
        self.canvas.create_rectangle(0, 730, 1040, 780, fill=theme["taskbar"], outline=theme["border"])

        # User Owner Pill on Taskbar
        self.canvas.create_rectangle(14, 738, 140, 772, fill=theme["accent"], outline="")
        self.canvas.create_text(77, 755, text=f"👤 {self.current_user}", fill="#000000", font=("Segoe UI", 9, "bold"))

        # Taskbar window tabs
        for idx, win in enumerate(self.windows):
            tx = 155 + idx * 160
            t_color = theme["titlebar_ina"] if idx == self.active_win_idx else theme["taskbar"]
            self.canvas.create_rectangle(tx, 738, tx + 150, 772, fill=t_color, outline=theme["border"])
            self.canvas.create_text(tx + 12, 755, text=win.title[:18], fill=theme["text"], font=("Segoe UI", 8), anchor="w")

        # Clock & Identity Tray (Right side)
        tray_text = f"Theme: {self.current_theme_name} · {self.current_hostname} (2026)"
        self.canvas.create_text(870, 755, text=tray_text, fill=theme["accent"], font=("Consolas", 8, "bold"))

    def _render_window_contents(self, win: WindowWidget, theme):
        # Window 0: OS Customizer Studio
        if "Customizer" in win.title:
            self.canvas.create_text(
                win.x + 20, win.y + 50,
                text="SYSTEM OWNERSHIP & THEME STUDIO\n"
                     "You have 100% root control over every visual component.\n",
                fill=theme["accent"], font=("Segoe UI", 9, "bold"), anchor="nw", tags=win.tag
            )

            self.canvas.create_text(
                win.x + 20, win.y + 90,
                text=f"Owner User : {self.current_user}\n"
                     f"Host System: {self.current_hostname}\n"
                     f"Active Skin: {self.current_theme_name} ({theme['tag']})\n\n"
                     "Click a theme button below to switch palette instantly:",
                fill=theme["text"], font=("Consolas", 8), anchor="nw", tags=win.tag
            )

            # Draw clickable theme switcher pills
            t_names = list(THEMES.keys())
            for idx, tname in enumerate(t_names):
                bx = win.x + 20 + (idx % 3) * 140
                by = win.y + 190 + (idx // 3) * 50
                is_cur = (tname == self.current_theme_name)
                btn_fill = theme["accent"] if is_cur else theme["win_bg"]
                btn_txt = "#000000" if is_cur else theme["text"]

                self.canvas.create_rectangle(bx, by, bx + 125, by + 34, fill=btn_fill, outline=theme["accent"], width=1, tags=win.tag)
                self.canvas.create_text(bx + 62, by + 17, text=tname, fill=btn_txt, font=("Segoe UI", 9, "bold"), tags=win.tag)

        # Window 1: Terminal with custom username
        elif "Terminal" in win.title:
            self.canvas.create_text(
                win.x + 18, win.y + 55,
                text=f"VortecoreOS 64-bit Kernel v1.4.0 (Customized Edition)\n"
                     f"Registered Owner: {self.current_user}@{self.current_hostname}\n"
                     f"Theme: {self.current_theme_name} (Full User Control Active)\n\n"
                     f"{self.current_user}@{self.current_hostname}> setuser {self.current_user}\n"
                     f"[CUSTOMIZER] Full OS ownership committed.\n"
                     f"{self.current_user}@{self.current_hostname}> theme {self.current_theme_name.lower()}\n"
                     f"[CUSTOMIZER] Palette applied in real-time.\n"
                     f"{self.current_user}@{self.current_hostname}> _",
                fill=theme["accent"], font=("Consolas", 9), anchor="nw", tags=win.tag
            )
        # Window 2: RTOS Monitor
        elif "Monitor" in win.title:
            self.canvas.create_text(
                win.x + 18, win.y + 55,
                text="HARDWARE ACCELERATION & CUSTOM TUNING\n"
                     "--------------------------------------------\n"
                     f"Current User   : {self.current_user} (Root Authority)\n"
                     f"Display Refresh: {theme['tag'].split()[-1]}\n"
                     "Window Shadow  : Enabled (Hardware Composited)\n"
                     "Input Polling  : ~0.1 ms (Raw PS/2 / USB HID)\n"
                     "IPC Latency    : 18 CPU Cycles (Lock-Free SPSC)\n"
                     "RAMFS Inodes   : Fully Writable by User\n"
                     "Privilege Mode : Custom Ring 3 Sandboxing Active",
                fill="#10B981", font=("Consolas", 9), anchor="nw", tags=win.tag
            )

    def _on_click(self, event):
        # 1. Check if clicked a theme button inside Customizer Window
        for win in self.windows:
            if "Customizer" in win.title:
                t_names = list(THEMES.keys())
                for idx, tname in enumerate(t_names):
                    bx = win.x + 20 + (idx % 3) * 140
                    by = win.y + 190 + (idx // 3) * 50
                    if bx <= event.x <= bx + 125 and by <= event.y <= by + 34:
                        self.current_theme_name = tname
                        self._render_desktop()
                        return

        # 2. Window selection & Dragging
        for idx in range(len(self.windows) - 1, -1, -1):
            win = self.windows[idx]
            if win.x <= event.x <= win.x + win.w and win.y <= event.y <= win.y + win.h:
                clicked_win = self.windows.pop(idx)
                self.windows.append(clicked_win)
                self.active_win_idx = len(self.windows) - 1

                if event.y <= clicked_win.y + 34:
                    self.dragged_win = clicked_win
                    self.drag_offset_x = event.x - clicked_win.x
                    self.drag_offset_y = event.y - clicked_win.y

                self._render_desktop()
                return

    def _on_drag(self, event):
        if self.dragged_win:
            self.dragged_win.x = max(0, min(1040 - self.dragged_win.w, event.x - self.drag_offset_x))
            self.dragged_win.y = max(0, min(730 - self.dragged_win.h, event.y - self.drag_offset_y))
            self._render_desktop()

    def _on_release(self, _event):
        self.dragged_win = None


if __name__ == "__main__":
    app = VortecoreDesktop()
    app.mainloop()
