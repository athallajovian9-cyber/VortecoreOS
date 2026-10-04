// =============================================================================
// VortecoreOS - Full User Customization & Ownership Engine (customizer.h)
// Allows the user to customize everything in real-time:
// - Hostname / User Identity / Prompt String
// - Desktop Theme & Palette (Accent colors, Titlebars, Backgrounds)
// - Font scale, Window opacity, Refresh rates (60Hz -> 240Hz)
// - Active system profiles and security level
// =============================================================================

#ifndef CUSTOMIZER_H
#define CUSTOMIZER_H

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

#define MAX_USERNAME_LEN 32
#define MAX_PROMPT_LEN   32
#define MAX_THEME_NAME   24

typedef struct {
    char     user_name[MAX_USERNAME_LEN];
    char     system_hostname[MAX_USERNAME_LEN];
    char     shell_prompt[MAX_PROMPT_LEN];
    char     theme_name[MAX_THEME_NAME];
    uint32_t color_desktop_bg;
    uint32_t color_taskbar_bg;
    uint32_t color_accent;
    uint32_t color_window_bg;
    uint32_t color_titlebar;
    uint32_t refresh_rate_hz;
    uint8_t  enable_window_shadows;
    uint8_t  enable_grid_wallpaper;
} user_custom_config_t;

void customizer_init(void);
user_custom_config_t* get_custom_config(void);
void set_theme_preset(const char* name);
void set_custom_identity(const char* user, const char* host);
void set_custom_prompt(const char* prompt);

#endif
