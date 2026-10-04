// =============================================================================
// VortecoreOS - Full User Customization Implementation (customizer.c)
// =============================================================================

#include "customizer.h"
#include "gui.h"

void kstrcpy(char* dest, const char* src);
int kstrcmp(const char* s1, const char* s2);

static user_custom_config_t config;

void customizer_init(void) {
    kstrcpy(config.user_name, "Athalla");
    kstrcpy(config.system_hostname, "vortecore-rig");
    kstrcpy(config.shell_prompt, "vortecore-x64> ");
    set_theme_preset("cyberpunk");
}

user_custom_config_t* get_custom_config(void) {
    return &config;
}

void set_theme_preset(const char* name) {
    kstrcpy(config.theme_name, name);

    if (kstrcmp(name, "cyberpunk") == 0) {
        config.color_desktop_bg = COLOR_RGB(10, 10, 16);     // Dark Void
        config.color_taskbar_bg = COLOR_RGB(18, 16, 28);     // Neon Dark
        config.color_accent     = COLOR_RGB(244, 63, 94);     // Neon Pink
        config.color_window_bg  = COLOR_RGB(21, 20, 32);     // Deep Purple-Gray
        config.color_titlebar   = COLOR_RGB(168, 85, 247);    // Vivid Purple
        config.refresh_rate_hz  = 240;
    }
    else if (kstrcmp(name, "matrix") == 0) {
        config.color_desktop_bg = COLOR_RGB(2, 8, 4);
        config.color_taskbar_bg = COLOR_RGB(4, 16, 8);
        config.color_accent     = COLOR_RGB(34, 197, 94);     // Matrix Green
        config.color_window_bg  = COLOR_RGB(6, 20, 10);
        config.color_titlebar   = COLOR_RGB(22, 163, 74);
        config.refresh_rate_hz  = 165;
    }
    else if (kstrcmp(name, "nord") == 0) {
        config.color_desktop_bg = COLOR_RGB(46, 52, 64);      // Polar Night
        config.color_taskbar_bg = COLOR_RGB(59, 66, 82);
        config.color_accent     = COLOR_RGB(136, 192, 208);   // Frost Blue
        config.color_window_bg  = COLOR_RGB(46, 52, 64);
        config.color_titlebar   = COLOR_RGB(129, 161, 193);
        config.refresh_rate_hz  = 144;
    }
    else if (kstrcmp(name, "solarized") == 0) {
        config.color_desktop_bg = COLOR_RGB(0, 43, 54);       // Solarized Base03
        config.color_taskbar_bg = COLOR_RGB(7, 54, 66);
        config.color_accent     = COLOR_RGB(181, 137, 0);     // Yellow
        config.color_window_bg  = COLOR_RGB(0, 43, 54);
        config.color_titlebar   = COLOR_RGB(38, 139, 210);    // Blue
        config.refresh_rate_hz  = 120;
    }
    else { // "obsidian" / default
        config.color_desktop_bg = COLOR_RGB(11, 14, 20);      // Obsidian
        config.color_taskbar_bg = COLOR_RGB(18, 22, 31);
        config.color_accent     = COLOR_RGB(56, 189, 248);    // Sky Blue
        config.color_window_bg  = COLOR_RGB(22, 27, 38);
        config.color_titlebar   = COLOR_RGB(34, 197, 94);     // Emerald
        config.refresh_rate_hz  = 144;
    }

    config.enable_window_shadows = 1;
    config.enable_grid_wallpaper = 1;
}

void set_custom_identity(const char* user, const char* host) {
    kstrcpy(config.user_name, user);
    kstrcpy(config.system_hostname, host);
}

void set_custom_prompt(const char* prompt) {
    kstrcpy(config.shell_prompt, prompt);
}
