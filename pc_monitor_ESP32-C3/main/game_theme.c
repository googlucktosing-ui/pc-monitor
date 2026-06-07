#include "game_theme.h"
#include "esp_log.h"
#include "app_state.h"
#include <stdio.h>
#include <string.h>

#include "game_theme/ui.h"

static const char *TAG = "GAME_THEME";
static bool s_created = false;

extern void ui_Screen1_screen_init(void);
extern void ui_Screen1_screen_destroy(void);

void game_theme_create(void)
{
    if (s_created) return;
    ui_Screen1_screen_init();
    if (!ui____initial_actions0) {
        ui____initial_actions0 = lv_obj_create(NULL);
    }
    lv_disp_load_scr(ui_Screen1);
    s_created = true;
    ESP_LOGI(TAG, "Game theme screen created");
}

void game_theme_destroy(void)
{
    if (!s_created) return;
    ui_destroy();
    s_created = false;
    ESP_LOGI(TAG, "Game theme screen destroyed");
}

void game_theme_refresh(void)
{
    if (!s_created) return;
    app_state_t st;
    app_state_get(&st);
    char buf[32];

    if (st.pc_connected && st.cpu_temp > 0) {
        snprintf(buf, sizeof(buf), "%.0f C", st.cpu_temp);
    } else { snprintf(buf, sizeof(buf), "-- C"); }
    lv_label_set_text(ui_cputempval, buf);

    if (st.pc_connected && st.gpu_temp > 0) {
        snprintf(buf, sizeof(buf), "%.0f C", st.gpu_temp);
    } else { snprintf(buf, sizeof(buf), "-- C"); }
    lv_label_set_text(ui_gputempval, buf);

    if (st.pc_connected) {
        snprintf(buf, sizeof(buf), "%d%%", (int)st.cpu_usage);
        lv_label_set_text(ui_perfval, buf);
        lv_label_set_text(ui_cpupct, buf);
        lv_arc_set_value(ui_cpu_arc, (int)st.cpu_usage);
        float freq = 800.0f + (st.cpu_usage / 100.0f) * 3200.0f;
        snprintf(buf, sizeof(buf), "%.1f GHz", freq / 1000.0f);
        lv_label_set_text(ui_cpufreq, buf);
    } else {
        lv_label_set_text(ui_perfval, "--%%");
        lv_label_set_text(ui_cpupct, "--%%");
        lv_arc_set_value(ui_cpu_arc, 0);
        lv_label_set_text(ui_cpufreq, "-- GHz");
    }

    if (st.pc_connected && st.gpu_usage > 0) {
        snprintf(buf, sizeof(buf), "%d%%", (int)st.gpu_usage);
        lv_label_set_text(ui_gpupct, buf);
        lv_arc_set_value(ui_gpu_arc, (int)st.gpu_usage);
        float gclk = 800.0f + (st.gpu_usage / 100.0f) * 1200.0f;
        snprintf(buf, sizeof(buf), "%.1f GHz", gclk / 1000.0f);
        lv_label_set_text(ui_gpufreq, buf);
    } else {
        lv_label_set_text(ui_gpupct, "--%%");
        lv_arc_set_value(ui_gpu_arc, 0);
        lv_label_set_text(ui_gpufreq, "-- GHz");
    }

    if (st.mem_total > 0) {
        int pct = (int)((st.mem_used / st.mem_total * 100) + 0.5f);
        snprintf(buf, sizeof(buf), "%.1f/%.1f GB", st.mem_used, st.mem_total);
        lv_label_set_text(ui_ramlabel, buf);
        lv_bar_set_value(ui_rambar, pct, LV_ANIM_OFF);
    } else {
        lv_label_set_text(ui_ramlabel, "--/-- GB");
        lv_bar_set_value(ui_rambar, 0, LV_ANIM_OFF);
    }

    if (st.pc_connected) {
        snprintf(buf, sizeof(buf), "DL" " %.1f MB/s", st.net_down);
        lv_label_set_text(ui_netdown, buf);
        snprintf(buf, sizeof(buf), "UL" " %.1f MB/s", st.net_up);
        lv_label_set_text(ui_netup, buf);
    } else {
        lv_label_set_text(ui_netdown, "DL" " -- MB/s");
        lv_label_set_text(ui_netup, "UL" " -- MB/s");
    }

    lv_label_set_text(ui_statusbar, st.pc_connected ? "PC CONNECTED" : "WAITING...");

    if (st.sensor_valid) {
        snprintf(buf, sizeof(buf), "%d.%d C %d%%",
                 st.indoor_temp_x10 / 10, st.indoor_temp_x10 % 10, st.indoor_hum);
        lv_label_set_text(ui_statusmid, buf);
    } else {
        lv_label_set_text(ui_statusmid, "--.-C --%");
    }
}
