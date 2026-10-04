// =============================================================================
// VortecoreOS - VESA VBE & High-Resolution Graphical Window Manager (gui.h)
// Architecture: Linear Framebuffer (LFB) Compositor with Dirty Rectangle Tracking,
//               PS/2 Mouse Driver, Floating Windows, Taskbar, and Event Dispatch
// =============================================================================

#ifndef GUI_H
#define GUI_H

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

#define SCREEN_WIDTH   1024
#define SCREEN_HEIGHT  768
#define SCREEN_BPP     32       // 32-bit ARGB TrueColor

#define MAX_WINDOWS    8
#define MAX_TITLE_LEN  32

// 32-bit Color Helper Macros
#define COLOR_ARGB(a, r, g, b) (((uint32_t)(a) << 24) | ((uint32_t)(r) << 16) | ((uint32_t)(g) << 8) | (uint32_t)(b))
#define COLOR_RGB(r, g, b)     COLOR_ARGB(255, r, g, b)

// Common Theme Colors
#define THEME_DESKTOP_BG    COLOR_RGB(11, 14, 20)       // Deep Obsidian
#define THEME_TASKBAR_BG    COLOR_RGB(18, 22, 31)       // Dark Slate
#define THEME_WINDOW_BG     COLOR_RGB(22, 27, 38)       // Glass Panel
#define THEME_TITLEBAR_ACT  COLOR_RGB(34, 197, 94)      // Emerald Green (Active)
#define THEME_TITLEBAR_INA  COLOR_RGB(45, 55, 72)       // Inactive
#define THEME_TEXT_WHITE    COLOR_RGB(248, 250, 252)
#define THEME_ACCENT_BLUE   COLOR_RGB(56, 189, 248)

// Window Structure (Compositor managed)
typedef struct {
    uint32_t id;
    int32_t  x, y;
    uint32_t width, height;
    char     title[MAX_TITLE_LEN];
    uint32_t bg_color;
    uint8_t  visible;
    uint8_t  active;
    uint8_t  is_terminal;
} window_t;

// PS/2 Mouse State
typedef struct {
    int32_t  x, y;
    uint8_t  left_button;
    uint8_t  right_button;
} mouse_state_t;

void gui_init(void);
void gui_draw_pixel(uint32_t x, uint32_t y, uint32_t color);
void gui_draw_rect(int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t color);
void gui_draw_text(int32_t x, int32_t y, const char* str, uint32_t color);
window_t* gui_create_window(const char* title, int32_t x, int32_t y, uint32_t w, uint32_t h);
void gui_render_frame(void);

#endif
