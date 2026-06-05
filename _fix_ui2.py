import re, os

with open("E:/CODEX-Pj/ESP32/pc_monitor_ESP32-C3/main/app_ui.c", encoding="utf-8") as f:
    c = f.read()

# Find the start of color defines
idx = c.find("#define CLR_BG")
idx_end = c.find("\n\n", idx)
while idx_end < len(c) and idx_end < idx + 600:
    next_idx = c.find("\n\n", idx_end + 2)
    if next_idx == -1: break
    # Check if the block between idx and next_idx+2 contains all color defines and nothing else
    block = c[idx:next_idx+2]
    if block.count("#define CLR_") >= 14:
        idx_end = next_idx + 2
        break
    idx_end = next_idx

old_block = c[idx:idx_end]

new_block = """/* ===== 3 themes =====
   0: dark blue (default)  1: aurora purple  2: cyber orange  */
static const lv_color_t _CLR[3][14] = {
    {   /* 0 */
        lv_color_hex(0x0F172A), lv_color_hex(0x1E293B), lv_color_hex(0xF1F5F9), lv_color_hex(0x94A3B8),
        lv_color_hex(0x3B82F6), lv_color_hex(0x8B5CF6), lv_color_hex(0x10B981), lv_color_hex(0xF59E0B),
        lv_color_hex(0xEF4444), lv_color_hex(0x22D3EE), lv_color_hex(0x22C55E), lv_color_hex(0x6B7280),
        lv_color_hex(0x334155), lv_color_hex(0xFCD34D),
    },
    {   /* 1 */
        lv_color_hex(0x0A0015), lv_color_hex(0x1A0A2E), lv_color_hex(0xF5F3FF), lv_color_hex(0x8B7EC8),
        lv_color_hex(0xA855F7), lv_color_hex(0xEC4899), lv_color_hex(0x06B6D4), lv_color_hex(0xF59E0B),
        lv_color_hex(0xFB923C), lv_color_hex(0x2DD4BF), lv_color_hex(0x22C55E), lv_color_hex(0x52525B),
        lv_color_hex(0x2A1A4E), lv_color_hex(0xFDE047),
    },
    {   /* 2 */
        lv_color_hex(0x0D0D0D), lv_color_hex(0x1C1C1C), lv_color_hex(0xF5F5F5), lv_color_hex(0x9CA3AF),
        lv_color_hex(0xF97316), lv_color_hex(0xEF4444), lv_color_hex(0x10B981), lv_color_hex(0xEAB308),
        lv_color_hex(0xF8714A), lv_color_hex(0x38BDF8), lv_color_hex(0x22C55E), lv_color_hex(0x5C5C5C),
        lv_color_hex(0x2A2A2A), lv_color_hex(0xFB923C),
    },
};
#define T_BG(i)       _CLR[s_theme_id][i]
#define CLR_BG       T_BG(0)
#define CLR_CARD_BG  T_BG(1)
#define CLR_TEXT     T_BG(2)
#define CLR_MUTED    T_BG(3)
#define CLR_CPU      T_BG(4)
#define CLR_GPU      T_BG(5)
#define CLR_MEM      T_BG(6)
#define CLR_DISK     T_BG(7)
#define CLR_NET_UP   T_BG(8)
#define CLR_NET_DOWN T_BG(9)
#define CLR_ON       T_BG(10)
#define CLR_OFF      T_BG(11)
#define CLR_LINE     T_BG(12)
#define CLR_WEATHER  T_BG(13)
"""

c = c[:idx] + new_block + c[idx_end:]
print("Palette replaced, size:", len(c))

# Add s_theme_id
c = c.replace("static lv_obj_t *s_pc_dht11_label;", "static lv_obj_t *s_pc_dht11_label;\n\n/* Theme state */\nstatic int s_theme_id = 0;")

# Update ui_task
old_task = """    lv_port_init();

    st7789_lcd_backlight(true);

    create_ui();"""
new_task = """    lv_port_init();

    st7789_lcd_backlight(true);

    /* Load initial theme */
    {
        app_state_t st;
        app_state_get(&st);
        s_theme_id = st.theme;
    }
    create_ui();"""
if old_task in c:
    c = c.replace(old_task, new_task)
    print("ui_task updated")
else:
    print("ERROR: ui_task pattern not found")

# Update ui_refresh
old_refresh = """static void ui_refresh(void)
{
    app_state_t st;
    app_state_get(&st);

    /* Auto-switch mode based on PC connection */

    switch_mode(!st.pc_connected);



    if (s_show_weather_clock) {

        refresh_weather_clock();

    } else {

        refresh_pc_monitor();

    }

}"""
new_refresh = """static void ui_refresh(void)
{
    app_state_t st;
    app_state_get(&st);

    /* Check for theme change */
    if (st.theme != s_theme_id) {
        s_theme_id = st.theme;
        lv_obj_clean(lv_scr_act());
        create_ui();
        return;
    }

    /* Auto-switch mode based on PC connection */

    switch_mode(!st.pc_connected);



    if (s_show_weather_clock) {

        refresh_weather_clock();

    } else {

        refresh_pc_monitor();

    }

}"""
if old_refresh in c:
    c = c.replace(old_refresh, new_refresh)
    print("ui_refresh updated")
else:
    print("ERROR: ui_refresh pattern not found")

with open("E:/CODEX-Pj/ESP32/pc_monitor_ESP32-C3/main/app_ui.c", "w", encoding="utf-8") as f:
    f.write(c)
print("app_ui.c OK", os.path.getsize("E:/CODEX-Pj/ESP32/pc_monitor_ESP32-C3/main/app_ui.c"), "bytes")