// =============================================================================
// VortecoreOS - Desktop Compositor & Window Manager Implementation (gui.c)
// =============================================================================

#include "gui.h"

void kstrcpy(char* dest, const char* src);
uint32_t kstrlen(const char* s);

static uint32_t* lfb_buffer = (uint32_t*)0xFD000000; // Simulated VBE Linear Framebuffer Base
static window_t windows[MAX_WINDOWS];
static uint32_t window_count = 0;
static mouse_state_t mouse = { .x = 512, .y = 384, .left_button = 0, .right_button = 0 };

void gui_draw_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (x < SCREEN_WIDTH && y < SCREEN_HEIGHT) {
        lfb_buffer[y * SCREEN_WIDTH + x] = color;
    }
}

void gui_draw_rect(int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t color) {
    for (int32_t row = 0; row < (int32_t)h; row++) {
        int32_t cur_y = y + row;
        if (cur_y < 0 || cur_y >= SCREEN_HEIGHT) continue;
        for (int32_t col = 0; col < (int32_t)w; col++) {
            int32_t cur_x = x + col;
            if (cur_x < 0 || cur_x >= SCREEN_WIDTH) continue;
            lfb_buffer[cur_y * SCREEN_WIDTH + cur_x] = color;
        }
    }
}

// 8x8 font rendering lookup
void gui_draw_char(int32_t x, int32_t y, char c, uint32_t color) {
    // Basic font block renderer
    for (int r = 0; r < 8; r++) {
        for (int col = 0; col < 6; col++) {
            if (c != ' ') {
                gui_draw_pixel((uint32_t)(x + col), (uint32_t)(y + r), color);
            }
        }
    }
}

void gui_draw_text(int32_t x, int32_t y, const char* str, uint32_t color) {
    int32_t cur_x = x;
    while (*str) {
        gui_draw_char(cur_x, y, *str++, color);
        cur_x += 8;
    }
}

window_t* gui_create_window(const char* title, int32_t x, int32_t y, uint32_t w, uint32_t h) {
    if (window_count >= MAX_WINDOWS) return 0;

    window_t* win = &windows[window_count++];
    win->id = window_count;
    win->x = x;
    win->y = y;
    win->width = w;
    win->height = h;
    win->bg_color = THEME_WINDOW_BG;
    win->visible = 1;
    win->active = (window_count == 1);
    kstrcpy(win->title, title);
    return win;
}

void gui_init(void) {
    window_count = 0;

    // 1. Create Terminal Window
    window_t* term = gui_create_window("Vortecore Terminal (x86_64)", 80, 80, 560, 380);
    term->is_terminal = 1;

    // 2. Create System Monitor / MogLinux Window
    gui_create_window("Vortecore Task Manager & RTOS Monitor", 420, 220, 520, 360);

    // 3. Create Filesystem Browser Window
    gui_create_window("RAMFS File Explorer", 120, 340, 440, 300);
}

void gui_render_frame(void) {
    // 1. Render Desktop Wallpaper
    gui_draw_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT - 48, THEME_DESKTOP_BG);

    // Subtle Grid pattern on wallpaper
    for (int y = 0; y < SCREEN_HEIGHT - 48; y += 40) {
        for (int x = 0; x < SCREEN_WIDTH; x += 40) {
            gui_draw_pixel((uint32_t)x, (uint32_t)y, COLOR_RGB(25, 30, 42));
        }
    }

    // 2. Render Windows in Z-order
    for (uint32_t i = 0; i < window_count; i++) {
        window_t* win = &windows[i];
        if (!win->visible) continue;

        // Window Drop Shadow
        gui_draw_rect(win->x + 6, win->y + 6, win->width, win->height, COLOR_ARGB(120, 0, 0, 0));

        // Window Body
        gui_draw_rect(win->x, win->y, win->width, win->height, win->bg_color);

        // Window Border
        gui_draw_rect(win->x, win->y, win->width, 1, COLOR_RGB(60, 70, 95));
        gui_draw_rect(win->x, win->y, 1, win->height, COLOR_RGB(60, 70, 95));
        gui_draw_rect(win->x + (int32_t)win->width - 1, win->y, 1, win->height, COLOR_RGB(60, 70, 95));
        gui_draw_rect(win->x, win->y + (int32_t)win->height - 1, win->width, 1, COLOR_RGB(60, 70, 95));

        // Window Titlebar (32px high)
        uint32_t title_color = win->active ? THEME_TITLEBAR_ACT : THEME_TITLEBAR_INA;
        gui_draw_rect(win->x + 1, win->y + 1, win->width - 2, 30, title_color);

        // Window Title text
        gui_draw_text(win->x + 12, win->y + 10, win->title, THEME_TEXT_WHITE);

        // Window Buttons (Close, Maximize, Minimize)
        gui_draw_rect(win->x + (int32_t)win->width - 24, win->y + 8, 14, 14, COLOR_RGB(239, 68, 68)); // Close Red
        gui_draw_rect(win->x + (int32_t)win->width - 44, win->y + 8, 14, 14, COLOR_RGB(234, 179, 8)); // Max Yellow
        gui_draw_rect(win->x + (int32_t)win->width - 64, win->y + 8, 14, 14, COLOR_RGB(34, 197, 94)); // Min Green
    }

    // 3. Render Modern Taskbar (Height 48px at bottom)
    int32_t tb_y = SCREEN_HEIGHT - 48;
    gui_draw_rect(0, tb_y, SCREEN_WIDTH, 48, THEME_TASKBAR_BG);
    gui_draw_rect(0, tb_y, SCREEN_WIDTH, 1, COLOR_RGB(45, 55, 75)); // Top border

    // Start / App Menu Pill
    gui_draw_rect(12, tb_y + 8, 110, 32, THEME_TITLEBAR_ACT);
    gui_draw_text(24, tb_y + 18, "VORTECORE", COLOR_RGB(10, 15, 20));

    // Active Task indicators
    for (uint32_t i = 0; i < window_count; i++) {
        int32_t pill_x = 135 + (int32_t)i * 140;
        uint32_t p_bg = windows[i].active ? COLOR_RGB(38, 48, 68) : COLOR_RGB(26, 32, 44);
        gui_draw_rect(pill_x, tb_y + 8, 130, 32, p_bg);
        gui_draw_text(pill_x + 10, tb_y + 18, windows[i].title, THEME_TEXT_WHITE);
    }

    // Clock & System Tray (Right side)
    gui_draw_text(SCREEN_WIDTH - 140, tb_y + 18, "x86_64  2026", THEME_ACCENT_BLUE);

    // 4. Render Mouse Pointer
    gui_draw_rect(mouse.x, mouse.y, 8, 8, COLOR_RGB(255, 255, 255));
}
