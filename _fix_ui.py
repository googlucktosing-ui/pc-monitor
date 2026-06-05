import re, os

with open("E:/CODEX-Pj/ESP32/pc_monitor_ESP32-C3/main/app_ui.c", encoding="utf-8") as f:
    c = f.read()

# Replace color palette with theme arrays
old_palette = """/* \xb7\xb1\xba\xcf\xba\xcf Color palette \xb7\xb1\xba\xcf\xba\xcf */

#define CLR_BG       lv_color_hex(0x0F172A)

#define CLR_CARD_BG  lv_color_hex(0x1E293B)

#define CLR_TEXT     lv_color_hex(0xF1F5F9)

#define CLR_MUTED    lv_color_hex(0x94A3B8)

#define CLR_CPU      lv_color_hex(0x3B82F6)

#define CLR_GPU      lv_color_hex(0x8B5CF6)

#define CLR_MEM      lv_color_hex(0x10B981)

#define CLR_DISK     lv_color_hex(0xF59E0B)

#define CLR_NET_UP   lv_color_hex(0xEF4444)

#define CLR_NET_DOWN lv_color_hex(0x22D3EE)

#define CLR_ON       lv_color_hex(0x22C55E)

#define CLR_OFF      lv_color_hex(0x6B7280)

#define CLR_LINE     lv_color_hex(0x334155)

#define CLR_WEATHER  lv_color_hex(0xFCD34D)"""

new_palette = """/* \xb7\xb1\xba\xcf\xba\xcf Color palette \xb7\xb1\xba\xcf\xba\xcf */

/* ===== 3 themes =====
   0: \xb0\xb5\xd2\xb9\xc0\xb6 (default)  1: \xbc\xab\xb9\xe2\xd7\xcf          2: \xc8\xfc\xb2\xa9\xb3\xd1        */
static const lv_color_t _CLR[3][14] = {
    {   /* 0: \xb0\xb5\xd2\xb9\xc0\xb6 - \xd7\xa8\xd2\xb5\xc9\xee\xc0\xb6 */
        lv_color_hex(0x0F172A), lv_color_hex(0x1E293B), lv_color_hex(0xF1F5F9), lv_color_hex(0x94A3B8),
        lv_color_hex(0x3B82F6), lv_color_hex(0x8B5CF6), lv_color_hex(0x10B981), lv_color_hex(0xF59E0B),
        lv_color_hex(0xEF4444), lv_color_hex(0x22D3EE), lv_color_hex(0x22C55E), lv_color_hex(0x6B7280),
        lv_color_hex(0x334155), lv_color_hex(0xFCD34D),
    },
    {   /* 1: \xbc\xab\xb9\xe2\xd7\xcf - \xc4\xe7\xba\xe7\xb7\xe7\xb8\xf1 */
        lv_color_hex(0x0A0015), lv_color_hex(0x1A0A2E), lv_color_hex(0xF5F3FF), lv_color_hex(0x8B7EC8),
        lv_color_hex(0xA855F7), lv_color_hex(0xEC4899), lv_color_hex(0x06B6D4), lv_color_hex(0xF59E0B),
        lv_color_hex(0xFB923C), lv_color_hex(0x2DD4BF), lv_color_hex(0x22C55E), lv_color_hex(0x52525B),
        lv_color_hex(0x2A1A4E), lv_color_hex(0xFDE047),
    },
    {   /* 2: \xc8\xfc\xb2\xa9\xb3\xd1 - \xb0\xb5\xc9\xab\xce\xc2\xc5\xaf */
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
#define CLR_WEATHER  T_BG(13)"""

if old_palette in c:
    c = c.replace(old_palette, new_palette)
else:
    print("ERROR: old palette not found!")
    # Try finding any CLR_ define
    idx = c.find("#define CLR_BG")
    print("Found CLR_BG at", idx)
    print(repr(c[idx:idx+400]))

# Add s_theme_id variable
c = c.replace("static lv_obj_t *s_pc_dht11_label;", "static lv_obj_t *s_pc_dht11_label;\n\n/* \xb7\xb1\xba\xcf\xba\xcf Theme state \xb7\xb1\xba\xcf\xba\xcf */\nstatic int s_theme_id = 0;")

# Update ui_task to load initial theme
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

c = c.replace(old_task, new_task)

# Update ui_refresh to check theme changes
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
        /* Rebuild UI with new theme */
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

c = c.replace(old_refresh, new_refresh)

with open("E:/CODEX-Pj/ESP32/pc_monitor_ESP32-C3/main/app_ui.c", "w", encoding="utf-8") as f:
    f.write(c)
print("app_ui.c OK", os.path.getsize("E:/CODEX-Pj/ESP32/pc_monitor_ESP32-C3/main/app_ui.c"), "bytes")